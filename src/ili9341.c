/**
 * Implementación mínima del driver ILI9341 para ESP32
 * ---------------------------------------------------
 *
 * Este archivo muestra, paso a paso, cómo hablar con una pantalla
 * ILI9341 usando el periférico SPI del ESP32 (ESP-IDF).
 *
 * Qué se cubre aquí:
 *  - Configuración del bus SPI (modo maestro).
 *  - Envío de comandos y datos al ILI9341.
 *  - Secuencia básica de inicialización del panel.
 *  - Primitivas para dibujar píxeles, rectángulos y texto simple.
 *
 * NO es un driver completo ni el más eficiente, pero es ideal para
 * entender el flujo básico y extenderlo en tu propio proyecto.
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "ili9341.h"

// TAG para logs relacionados con la pantalla
static const char *TAG = "ILI9341_DRV";

// -----------------------------------------------------------------------------
//  CONFIGURACIÓN DE PINES (ADÁPTALA A TU HARDWARE)
// -----------------------------------------------------------------------------

/**
 * Conexiones típicas del módulo ILI9341 con ESP32:
 *
 *   ESP32        ->   ILI9341
 *   ---------------------------
 *   MOSI (GPIO23) -> SDI (MOSI)
 *   MISO (GPIO19) -> SDO (MISO)  [opcional si no lees del panel]
 *   SCK  (GPIO18) -> SCK
 *   CS   (GPIO5)  -> CS
 *   DC   (GPIO2)  -> D/C (Data/Command)
 *   RST  (GPIO4)  -> RESET
 *   BL   (GPIO15) -> LED / BL (Backlight) [a veces va directo a 3V3]
 */

#define ILI9341_PIN_MISO   19
#define ILI9341_PIN_MOSI   23
#define ILI9341_PIN_CLK    18
#define ILI9341_PIN_CS     15
#define ILI9341_PIN_DC     2
#define ILI9341_PIN_RST    4
#define ILI9341_PIN_BL     -1


// Se usará el host VSPI (SPI3) del ESP32
#define ILI9341_SPI_HOST   HSPI_HOST

// Handle global del dispositivo SPI asociado a la pantalla
static spi_device_handle_t ili9341_spi;

// -----------------------------------------------------------------------------
//  AYUDANTES INTERNOS: GPIO Y SPI BÁSICO
// -----------------------------------------------------------------------------

/**
 * Envía un comando (byte) al ILI9341.
 * DC = 0 indica "COMMAND".
 */
static void ili9341_send_cmd(uint8_t cmd)
{
    gpio_set_level(ILI9341_PIN_DC, 0);  // Modo comando

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;           // 8 bits
    t.tx_buffer = &cmd;

    esp_err_t ret = spi_device_transmit(ili9341_spi, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error enviando comando 0x%02X", cmd);
    }
}

/**
 * Envía un bloque de datos al ILI9341.
 * DC = 1 indica "DATA".
 */
static void ili9341_send_data(const uint8_t *data, int len)
{
    if (len <= 0) return;

    gpio_set_level(ILI9341_PIN_DC, 1);  // Modo datos

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = len * 8;   // longitud en bits
    t.tx_buffer = data;

    esp_err_t ret = spi_device_transmit(ili9341_spi, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error enviando datos SPI (%d bytes)", len);
    }
}

/**
 * Envía un solo byte de datos (atajo frecuente).
 */
static void ili9341_send_data8(uint8_t data)
{
    ili9341_send_data(&data, 1);
}

/**
 * Envía un valor de 16 bits (por ejemplo, un color RGB565) como 2 bytes.
 */
static void ili9341_send_data16(uint16_t data)
{
    uint8_t buf[2];
    buf[0] = (data >> 8) & 0xFF;  // byte alto
    buf[1] = data & 0xFF;         // byte bajo
    ili9341_send_data(buf, 2);
}

// -----------------------------------------------------------------------------
//  COMANDOS BÁSICOS ILI9341
// -----------------------------------------------------------------------------

// Algunos comandos usados en la secuencia de inicio

#define ILI9341_CMD_SWRESET  0x01
#define ILI9341_CMD_SLPOUT   0x11
#define ILI9341_CMD_DISPON   0x29
#define ILI9341_CMD_CASET    0x2A
#define ILI9341_CMD_RASET    0x2B
#define ILI9341_CMD_RAMWR    0x2C
#define ILI9341_CMD_MADCTL   0x36
#define ILI9341_CMD_PIXFMT   0x3A

// MADCTL (Memory Access Control) bits para orientación
#define MADCTL_MX  0x40
#define MADCTL_MY  0x80
#define MADCTL_MV  0x20
#define MADCTL_BGR 0x08

// -----------------------------------------------------------------------------
//  CONFIGURACIÓN DE VENTANA DE DIBUJO
// -----------------------------------------------------------------------------

/**
 * Define el área (ventana) de memoria de vídeo donde se va a escribir.
 * Todo lo que se envíe con RAMWR a continuación rellenará ese rectángulo.
 */
static void ili9341_set_address_window(uint16_t x0, uint16_t y0,
                                       uint16_t x1, uint16_t y1)
{
    // Columna (X)
    ili9341_send_cmd(ILI9341_CMD_CASET);
    uint8_t data_col[4] = {
        (uint8_t)(x0 >> 8), (uint8_t)(x0 & 0xFF),
        (uint8_t)(x1 >> 8), (uint8_t)(x1 & 0xFF)
    };
    ili9341_send_data(data_col, 4);

    // Fila (Y)
    ili9341_send_cmd(ILI9341_CMD_RASET);
    uint8_t data_row[4] = {
        (uint8_t)(y0 >> 8), (uint8_t)(y0 & 0xFF),
        (uint8_t)(y1 >> 8), (uint8_t)(y1 & 0xFF)
    };
    ili9341_send_data(data_row, 4);

    // Siguiente comando escribirá en esta ventana
    ili9341_send_cmd(ILI9341_CMD_RAMWR);
}

// -----------------------------------------------------------------------------
//  INICIALIZACIÓN DEL PANEL
// -----------------------------------------------------------------------------

/**
 * Inicializa el bus SPI, configura los pines GPIO y envía
 * una secuencia básica de arranque al controlador ILI9341.
 */
void ili9341_init(void)
{
    // 1. Configuración de pines de control como salidas
    gpio_config_t io_conf = {0};
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;

    // Pines DC, RST, BL y CS
    io_conf.pin_bit_mask = (1ULL << ILI9341_PIN_DC) |
                           (1ULL << ILI9341_PIN_RST) |
                           
                           (1ULL << ILI9341_PIN_CS);
    gpio_config(&io_conf);

    // Inicializamos niveles por defecto
    gpio_set_level(ILI9341_PIN_CS, 1);   // CS inactivo (alto)
    gpio_set_level(ILI9341_PIN_DC, 0);
    gpio_set_level(ILI9341_PIN_RST, 1);
       // Encender backlight

    // 2. Inicializar el bus SPI
    spi_bus_config_t buscfg = {
        .miso_io_num = ILI9341_PIN_MISO,
        .mosi_io_num = ILI9341_PIN_MOSI,
        .sclk_io_num = ILI9341_PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = ILI9341_WIDTH * ILI9341_HEIGHT * 2 + 8
    };

    esp_err_t ret = spi_bus_initialize(ILI9341_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error en spi_bus_initialize: %d", ret);
        return;
    }

    // 3. Añadir el dispositivo (la pantalla) al bus SPI
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 40 * 1000 * 1000, // 40 MHz (puedes bajar si tu cableado es malo)
        .mode = 0,                          // Modo SPI 0
        .spics_io_num = ILI9341_PIN_CS,     // CS gestionado por el driver
        .queue_size = 7,
        .flags = SPI_DEVICE_HALFDUPLEX,
    };

    ret = spi_bus_add_device(ILI9341_SPI_HOST, &devcfg, &ili9341_spi);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error en spi_bus_add_device: %d", ret);
        return;
    }

    // 4. Reset por hardware del panel
    gpio_set_level(ILI9341_PIN_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(ILI9341_PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(150));

    // 5. Secuencia de inicialización mínima según hoja de datos
    ili9341_send_cmd(ILI9341_CMD_SWRESET);   // Software reset
    vTaskDelay(pdMS_TO_TICKS(120));

    ili9341_send_cmd(ILI9341_CMD_SLPOUT);    // Salir de modo SLEEP
    vTaskDelay(pdMS_TO_TICKS(120));

    // Formato de píxel: 16 bits/píxel (RGB565)
    ili9341_send_cmd(ILI9341_CMD_PIXFMT);
    ili9341_send_data8(0x55); // 16 bits/pixel

    // Memory Access Control: orientación y modo BGR
    ili9341_send_cmd(ILI9341_CMD_MADCTL);
    // MY | MX | BGR  => orientación vertical típica
    uint8_t madctl = MADCTL_MY | MADCTL_MX | MADCTL_BGR;
    ili9341_send_data8(madctl);

    // Encender la pantalla
    ili9341_send_cmd(ILI9341_CMD_DISPON);
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "ILI9341 inicializado");

    // Dibuja un cuadrado negro de 80x80 en el centro de la pantalla
    uint16_t square_size = 80;
    uint16_t x0 = (ILI9341_WIDTH - square_size) / 2;
    uint16_t y0 = (ILI9341_HEIGHT - square_size) / 2;
    ili9341_fill_rect(x0, y0, square_size, square_size, 0x0000); // 0x0000 = negro
}

// -----------------------------------------------------------------------------
//  PRIMITIVAS DE DIBUJO
// -----------------------------------------------------------------------------

void ili9341_fill_screen(uint16_t color)
{
    // Establecemos toda la pantalla como ventana de dibujo
    ili9341_set_address_window(0, 0, ILI9341_WIDTH - 1, ILI9341_HEIGHT - 1);

    // Enviamos color repetido para todos los píxeles
    uint32_t total_pixels = (uint32_t)ILI9341_WIDTH * (uint32_t)ILI9341_HEIGHT;

    // Para no crear un buffer enorme, escribimos en bloques
    const int block_size = 1024; // píxeles por bloque
    uint16_t block[block_size];
    for (int i = 0; i < block_size; i++) {
        block[i] = color;
    }

    while (total_pixels > 0) {
        uint32_t to_write = (total_pixels > block_size) ? block_size : total_pixels;
        ili9341_send_data((uint8_t *)block, to_write * 2);
        total_pixels -= to_write;
    }
}

void ili9341_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT) {
        return; // fuera de rango
    }

    ili9341_set_address_window(x, y, x, y);
    ili9341_send_data16(color);
}

void ili9341_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    // Clipping muy básico para evitar salir de la pantalla
    if (x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT) return;

    if ((x + w) > ILI9341_WIDTH)  w = ILI9341_WIDTH - x;
    if ((y + h) > ILI9341_HEIGHT) h = ILI9341_HEIGHT - y;

    ili9341_set_address_window(x, y, x + w - 1, y + h - 1);

    uint32_t total_pixels = (uint32_t)w * (uint32_t)h;
    const int block_size = 1024;
    uint16_t block[block_size];
    for (int i = 0; i < block_size; i++) {
        block[i] = color;
    }

    while (total_pixels > 0) {
        uint32_t to_write = (total_pixels > block_size) ? block_size : total_pixels;
        ili9341_send_data((uint8_t *)block, to_write * 2);
        total_pixels -= to_write;
    }
}

// -----------------------------------------------------------------------------
//  FUENTE 5x7 BÁSICA PARA TEXTO
// -----------------------------------------------------------------------------

/**
 * Para simplificar y evitar una fuente completa, definimos un pequeño
 * subconjunto de caracteres necesarios para el minijuego (A, E, F, I,
 * L, N, O, P, R, S, T, U, espacio, dígitos 0-9, etc.).
 *
 * Cada carácter es una matriz 5x7, almacenada como 5 columnas de 7 bits.
 * El bit menos significativo es la fila superior.
 */

typedef struct {
    char ch;           // carácter ASCII
    uint8_t columns[5]; // 5 columnas, 7 bits útiles cada una
} font5x7_char_t;

// Pequeño set de caracteres: " .": espacio, letras usadas, dígitos.
// El diseño de cada letra es aproximado; puedes modificarlos a tu gusto.

static const font5x7_char_t font5x7_table[] = {
    // ESPACIO ' '
    { ' ', { 0x00, 0x00, 0x00, 0x00, 0x00 } },

    // DÍGITOS 0-9
    { '0', { 0x3E, 0x51, 0x49, 0x45, 0x3E } }, // 0
    { '1', { 0x00, 0x42, 0x7F, 0x40, 0x00 } }, // 1
    { '2', { 0x42, 0x61, 0x51, 0x49, 0x46 } }, // 2
    { '3', { 0x21, 0x41, 0x45, 0x4B, 0x31 } }, // 3
    { '4', { 0x18, 0x14, 0x12, 0x7F, 0x10 } }, // 4
    { '5', { 0x27, 0x45, 0x45, 0x45, 0x39 } }, // 5
    { '6', { 0x3C, 0x4A, 0x49, 0x49, 0x30 } }, // 6
    { '7', { 0x01, 0x71, 0x09, 0x05, 0x03 } }, // 7
    { '8', { 0x36, 0x49, 0x49, 0x49, 0x36 } }, // 8
    { '9', { 0x06, 0x49, 0x49, 0x29, 0x1E } }, // 9

    // LETRAS BÁSICAS (mayúsculas)
    { 'A', { 0x7E, 0x11, 0x11, 0x11, 0x7E } },
    { 'C', { 0x3E, 0x41, 0x41, 0x41, 0x22 } },
    { 'E', { 0x7F, 0x49, 0x49, 0x49, 0x41 } },
    { 'F', { 0x7F, 0x09, 0x09, 0x09, 0x01 } },
    { 'H', { 0x7F, 0x08, 0x08, 0x08, 0x7F } },
    { 'I', { 0x00, 0x41, 0x7F, 0x41, 0x00 } },
    { 'L', { 0x7F, 0x40, 0x40, 0x40, 0x40 } },
    { 'M', { 0x7F, 0x02, 0x0C, 0x02, 0x7F } },
    { 'N', { 0x7F, 0x04, 0x08, 0x10, 0x7F } },
    { 'O', { 0x3E, 0x41, 0x41, 0x41, 0x3E } },
    { 'P', { 0x7F, 0x09, 0x09, 0x09, 0x06 } },
    { 'R', { 0x7F, 0x09, 0x19, 0x29, 0x46 } },
    { 'S', { 0x46, 0x49, 0x49, 0x49, 0x31 } },
    { 'T', { 0x01, 0x01, 0x7F, 0x01, 0x01 } },
    { 'U', { 0x3F, 0x40, 0x40, 0x40, 0x3F } },
    { ':', { 0x00, 0x36, 0x36, 0x00, 0x00 } },
    { '.', { 0x00, 0x40, 0x60, 0x00, 0x00 } },
};

// Número de elementos en la tabla
static const int FONT5X7_TABLE_LEN = sizeof(font5x7_table) / sizeof(font5x7_table[0]);

/**
 * Busca la definición bitmap del carácter indicado.
 * Si no se encuentra, devuelve la de espacio.
 */
static const font5x7_char_t *font5x7_find(char ch)
{
    for (int i = 0; i < FONT5X7_TABLE_LEN; i++) {
        if (font5x7_table[i].ch == ch) {
            return &font5x7_table[i];
        }
    }
    // Fallback: espacio
    return &font5x7_table[0];
}

void ili9341_draw_char(uint16_t x, uint16_t y, char ch,
                       uint16_t color, uint16_t bg, uint8_t scale)
{
    const font5x7_char_t *glyph = font5x7_find(ch);

    // Dimensiones del carácter base
    const uint8_t char_w = 5;
    const uint8_t char_h = 7;

    uint16_t scaled_w = char_w * scale;
    uint16_t scaled_h = char_h * scale;

    // Pintar un fondo sólido para el carácter completo
    ili9341_fill_rect(x, y, scaled_w, scaled_h, bg);

    // Recorremos columnas y filas del bitmap
    for (uint8_t col = 0; col < char_w; col++) {
        uint8_t column_bits = glyph->columns[col];
        for (uint8_t row = 0; row < char_h; row++) {
            if (column_bits & (1 << row)) {
                // Este píxel pertenece al trazo del carácter
                uint16_t px = x + col * scale;
                uint16_t py = y + row * scale;

                // Dibujar un bloque scale x scale para dar grosor
                ili9341_fill_rect(px, py, scale, scale, color);
            }
        }
    }
}

void ili9341_draw_string(uint16_t x, uint16_t y, const char *text,
                         uint16_t color, uint16_t bg, uint8_t scale)
{
    // Avance horizontal entre caracteres (6 columnas: 5 + 1 espacio)
    uint16_t advance = 6 * scale;

    uint16_t cursor_x = x;
    const char *p = text;
    while (*p) {
        char ch = *p;
        if (ch == '\n') {
            // Salto de línea manual sencillo
            cursor_x = x;
            y += (8 * scale); // 7 filas + 1 de espacio
        } else {
            ili9341_draw_char(cursor_x, y, ch, color, bg, scale);
            cursor_x += advance;
        }
        p++;
    }
}
