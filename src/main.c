/**
 * Stratagem Hero (MVP) para ESP32 + ILI9341
 * -----------------------------------------
 *
 * Este archivo es un TEMPLATE didáctico, pensado para que entiendas:
 *  - Cómo se inicializa el ESP32 (FreeRTOS, app_main).
 *  - Cómo se configura el bus SPI y el driver de una pantalla ILI9341.
 *  - Cómo leer 4 botones físicos (↑ ↓ ← →).
 *  - Cómo estructurar un minijuego con máquina de estados.
 *
 * IMPORTANTE:
 *  - TODO está escrito en C (no C++).
 *  - El driver ILI9341 implementado es muy básico: suficiente para
 *    dibujar fondo, rectángulos y texto con una fuente simple.
 *  - Este código está pensado como punto de partida para que lo
 *    modifiques y amplíes en tu propio proyecto.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"     // Para obtener tiempo en microsegundos
#include "esp_system.h"    // esp_random()
#include "esp_random.h"

#include "ili9341.h"       // Nuestro driver de pantalla (en C)

// TAG para logs por puerto serie
static const char *TAG = "STRATAGEM_HERO";

// -----------------------------------------------------------------------------
//  Configuración de pines para los BOTONES (ajusta a tu hardware)
// -----------------------------------------------------------------------------
// Se asume: botones conectados a GND y GPIO con resistencia PULLUP interna.
//  - Cuando NO se pulsa -> nivel lógico 1.
//  - Cuando se pulsa    -> nivel lógico 0.

#define BTN_UP_GPIO     GPIO_NUM_32
#define BTN_DOWN_GPIO   GPIO_NUM_33
#define BTN_LEFT_GPIO   GPIO_NUM_25
#define BTN_RIGHT_GPIO  GPIO_NUM_26

// -----------------------------------------------------------------------------
//  Definiciones del JUEGO
// -----------------------------------------------------------------------------

// Direcciones posibles (coinciden con el enunciado)
typedef enum {
    DIR_UP = 0,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

// Máquina de estados del minijuego
typedef enum {
    GAME_MENU_INIT = 0,  // Mostrar "Pulsa para empezar"
    GAME_GEN_SEQ,        // Generar secuencia aleatoria
    GAME_SHOW_SEQ,       // Mostrar secuencia al jugador
    GAME_WAIT_INPUT,     // Leer direcciones y compararlas
    GAME_RESULT          // Mostrar ÉXITO / FALLO
} GameState;

// Tamaño máximo de la secuencia
#define MAX_SEQ_LENGTH   6

// Límite mínimo y máximo de longitud de secuencia
#define MIN_SEQ_LENGTH   3
#define MAX_SEQ_LENGTH   6

// Tiempo máximo para introducir la secuencia (en milisegundos)
#define INPUT_TIME_LIMIT_MS  10000  // 10 s

// para UI (layout muy simple)
#define COLOR_BG      ILI9341_COLOR_BLACK
#define COLOR_TEXT    ILI9341_COLOR_WHITE
#define COLOR_GOOD    ILI9341_COLOR_GREEN
#define COLOR_BAD     ILI9341_COLOR_RED
#define COLOR_INFO    ILI9341_COLOR_YELLOW

// Área de texto principal
#define TEXT_LINE_HEIGHT   16  // depende del tamaño de fuente 8x8 escalada x2

// -----------------------------------------------------------------------------
//  PROTOTIPOS de funciones internas
// -----------------------------------------------------------------------------

static void buttons_init(void);
static int  button_is_pressed(gpio_num_t gpio_num);
static Direction wait_for_any_direction(TickType_t timeout_ticks, int *pressed);

static void game_draw_menu_screen(void);
static void game_draw_sequence(Direction *seq, int length);
static void game_draw_input_progress(Direction *seq, int length, int current_index);
static void game_draw_timer(uint32_t remaining_ms);
static void game_draw_result(int success);

static const char *direction_to_char(Direction d);

// -----------------------------------------------------------------------------
//  INICIALIZACIÓN DE HARDWARE Y ARRANQUE DEL JUEGO
// -----------------------------------------------------------------------------

/**
 * app_main
 * --------
 * Punto de entrada en ESP-IDF. Aquí:
 *  - Inicializamos logs.
 *  - Inicializamos la pantalla ILI9341.
 *  - Configuramos los pines de los botones.
 *  - Entramos en el bucle principal del minijuego.
 */
void app_main(void)
{
    // Pequeño retardo al arranque (suele ser útil al flashear)
    vTaskDelay(pdMS_TO_TICKS(5000));

    ESP_LOGI(TAG, "Iniciando Stratagem Hero (MVP)...");

    // 1. Inicializar pantalla ILI9341 (SPI + comandos de inicio)
    ili9341_init();

    // Limpiar pantalla con un color de fondo
    ili9341_fill_screen(COLOR_BG);

    // 2. Inicializar botones
    buttons_init();

    // 3. Variables del juego
    Direction sequence[MAX_SEQ_LENGTH];        // Secuencia objetivo
    int seq_length = 3;                        // Empezamos con 3 pasos
    GameState state = GAME_MENU_INIT;          // Estado inicial
    int running = 1;                           // Por si quisieras salir algún día

    // 4. Bucle principal del juego (máquina de estados)
    while (running) {
        switch (state) {
        case GAME_MENU_INIT: {
            // Pantalla de bienvenida
            game_draw_menu_screen();

            ESP_LOGI(TAG, "Esperando que el jugador pulse cualquier dirección...");

            // Espera bloqueante hasta que pulse cualquiera de los cuatro botones
            int pressed = 0;
            (void)wait_for_any_direction(portMAX_DELAY, &pressed);
            if (pressed) {
                // Siguiente estado: generar nueva secuencia
                state = GAME_GEN_SEQ;
            }
            break;
        }

        case GAME_GEN_SEQ: {
            // Longitud aleatoria entre MIN_SEQ_LENGTH y MAX_SEQ_LENGTH
            uint32_t r = esp_random();
            seq_length = MIN_SEQ_LENGTH + (r % (MAX_SEQ_LENGTH - MIN_SEQ_LENGTH + 1));

            ESP_LOGI(TAG, "Generando secuencia de longitud %d", seq_length);

            for (int i = 0; i < seq_length; i++) {
                // esp_random() devuelve 32 bits, tomamos los 2 LSB para obtener [0..3]
                sequence[i] = (Direction)(esp_random() % 4);
            }

            state = GAME_SHOW_SEQ;
            break;
        }

        case GAME_SHOW_SEQ: {
            // Mostramos la secuencia para que el jugador la memorice
            ESP_LOGI(TAG, "Mostrando secuencia al jugador");

            ili9341_fill_screen(COLOR_BG);

            // Texto cabecera
            ili9341_draw_string(10, 10, "SECUENCIA:", COLOR_INFO, COLOR_BG, 2);

            // Dibuja la secuencia como letras U, D, L, R
            game_draw_sequence(sequence, seq_length);

            // Pausa unos segundos para que el jugador la vea
            vTaskDelay(pdMS_TO_TICKS(2000));

            // Limpia parcialmente para el modo de input
            ili9341_fill_screen(COLOR_BG);
            ili9341_draw_string(10, 10, "INTRODUCE LA SECUENCIA", COLOR_INFO, COLOR_BG, 2);
            ili9341_draw_string(10, 10 + TEXT_LINE_HEIGHT * 2, "TU INPUT:", COLOR_TEXT, COLOR_BG, 2);

            state = GAME_WAIT_INPUT;
            break;
        }

        case GAME_WAIT_INPUT: {
            ESP_LOGI(TAG, "Esperando entradas del jugador...");

            int current_index = 0;    // Progreso dentro de sequence[]
            int success = 1;          // Suponemos éxito hasta que falle

            // Tiempo de inicio (en microsegundos)
            int64_t start_us = esp_timer_get_time();
            int64_t limit_us = (int64_t)INPUT_TIME_LIMIT_MS * 1000;

            while (current_index < seq_length) {
                int64_t now_us = esp_timer_get_time();
                int64_t elapsed_us = now_us - start_us;

                if (elapsed_us >= limit_us) {
                    // Se acabó el tiempo
                    ESP_LOGW(TAG, "Tiempo agotado");
                    success = 0;
                    break;
                }

                // Tiempo restante en ms (para dibujar temporizador)
                uint32_t remaining_ms = (uint32_t)((limit_us - elapsed_us) / 1000);
                game_draw_timer(remaining_ms);

                // Esperamos a que se pulse algún botón, pero con timeout pequeño
                int pressed = 0;
                Direction d = wait_for_any_direction(pdMS_TO_TICKS(50), &pressed);
                if (!pressed) {
                    // No se ha pulsado nada en este slice de 50 ms, seguimos
                    continue;
                }

                // Comprobamos si coincide con la secuencia objetivo
                if (d == sequence[current_index]) {
                    ESP_LOGI(TAG, "Paso %d correcto", current_index + 1);
                    current_index++;
                    // Actualizamos la representación gráfica del input
                    game_draw_input_progress(sequence, seq_length, current_index);
                } else {
                    ESP_LOGI(TAG, "Paso %d INCORRECTO", current_index + 1);
                    success = 0;
                    break;
                }
            }

            // Hemos salido del bucle: o completó secuencia, o fallo, o timeout
            game_draw_result(success && (current_index == seq_length));

            // Pequeña pausa antes de volver al menú
            vTaskDelay(pdMS_TO_TICKS(2500));

            state = GAME_MENU_INIT;
            break;
        }

        case GAME_RESULT:
            // En esta implementación, la lógica de resultado se maneja
            // directamente dentro de GAME_WAIT_INPUT, por simplicidad.
            state = GAME_MENU_INIT;
            break;
        }
    }
}

// -----------------------------------------------------------------------------
//  IMPLEMENTACIÓN: BOTONES FÍSICOS
// -----------------------------------------------------------------------------

/**
 * Configura los GPIO de los 4 botones como entradas con PULLUP interno.
 */
static void buttons_init(void)
{
    gpio_config_t io_conf = {0};
    io_conf.intr_type = GPIO_INTR_DISABLE;       // Sin interrupciones (polling)
    io_conf.mode = GPIO_MODE_INPUT;              // Modo entrada
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;     // Activar PULLUP

    // Máscara de pines (OR de los 4)
    io_conf.pin_bit_mask = (1ULL << BTN_UP_GPIO) |
                           (1ULL << BTN_DOWN_GPIO) |
                           (1ULL << BTN_LEFT_GPIO) |
                           (1ULL << BTN_RIGHT_GPIO);

    gpio_config(&io_conf);
}

/**
 * Devuelve 1 si el botón (GPIO) está pulsado, 0 en caso contrario.
 *
 * Recuerda: botón a GND con PULLUP -> pulsado = nivel 0 lógico.
 */
static int button_is_pressed(gpio_num_t gpio_num)
{
    int level = gpio_get_level(gpio_num);
    return (level == 0); // 0 = pulsado
}

/**
 * Espera a que se pulse alguno de los 4 botones direccionales.
 *
 *  - timeout_ticks: tiempo máximo a esperar en ticks de FreeRTOS.
 *                   Si es portMAX_DELAY, espera indefinidamente.
 *  - pressed: puntero de salida, vale 1 si se ha pulsado
 *             algún botón antes de timeout, 0 si no.
 *
 * Devuelve: la Direction correspondiente si se ha pulsado algo;
 *           si no, devuelve DIR_UP por defecto (pero pressed = 0).
 */
static Direction wait_for_any_direction(TickType_t timeout_ticks, int *pressed)
{
    TickType_t start = xTaskGetTickCount();

    if (pressed) {
        *pressed = 0;
    }

    while (1) {
        // Comprobamos cada botón. Si hay varios pulsados, devolvemos el primero
        if (button_is_pressed(BTN_UP_GPIO)) {
            // Sencillo "debounce": esperamos a que se suelte antes de salir
            while (button_is_pressed(BTN_UP_GPIO)) {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            if (pressed) *pressed = 1;
            return DIR_UP;
        }
        if (button_is_pressed(BTN_DOWN_GPIO)) {
            while (button_is_pressed(BTN_DOWN_GPIO)) {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            if (pressed) *pressed = 1;
            return DIR_DOWN;
        }
        if (button_is_pressed(BTN_LEFT_GPIO)) {
            while (button_is_pressed(BTN_LEFT_GPIO)) {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            if (pressed) *pressed = 1;
            return DIR_LEFT;
        }
        if (button_is_pressed(BTN_RIGHT_GPIO)) {
            while (button_is_pressed(BTN_RIGHT_GPIO)) {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            if (pressed) *pressed = 1;
            return DIR_RIGHT;
        }

        // Nadie está pulsado -> comprobamos timeout
        if (timeout_ticks != portMAX_DELAY) {
            TickType_t now = xTaskGetTickCount();
            if ((now - start) >= timeout_ticks) {
                // Timeout alcanzado
                if (pressed) *pressed = 0;
                return DIR_UP;  // valor por defecto, irrelevante si pressed=0
            }
        }

        // Pequeño retardo para no saturar la CPU
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// -----------------------------------------------------------------------------
//  IMPLEMENTACIÓN: DIBUJO DE UI DEL JUEGO SOBRE ILI9341
// -----------------------------------------------------------------------------

/**
 * Dibuja la pantalla de menú inicial:
 *  - Título del juego.
 *  - Instrucciones mínimas.
 */
static void game_draw_menu_screen(void)
{
    ili9341_fill_screen(COLOR_BG);

    int y = 20;
    ili9341_draw_string(10, y, "STRATAGEM HERO", COLOR_INFO, COLOR_BG, 2); y += TEXT_LINE_HEIGHT * 2;
    ili9341_draw_string(10, y, "MVP ESP32 + ILI9341", COLOR_TEXT, COLOR_BG, 1); y += TEXT_LINE_HEIGHT * 2;

    ili9341_draw_string(10, y, "Pulsa cualquier flecha", COLOR_TEXT, COLOR_BG, 1); y += TEXT_LINE_HEIGHT;
    ili9341_draw_string(10, y, "para empezar", COLOR_TEXT, COLOR_BG, 1);
}

/**
 * Dibuja la secuencia de direcciones generada (como letras U, D, L, R)
 * en una única línea.
 */
static void game_draw_sequence(Direction *seq, int length)
{
    int x = 10;
    int y = 10 + TEXT_LINE_HEIGHT * 2;  // debajo de "SECUENCIA:"

    for (int i = 0; i < length; i++) {
        const char *s = direction_to_char(seq[i]);
        ili9341_draw_string(x, y, s, COLOR_TEXT, COLOR_BG, 2);
        x += 20; // separador horizontal
    }
}

/**
 * Dibuja el progreso del jugador en la línea "TU INPUT:".
 * Muestra todas las flechas de la secuencia, y pinta en verde
 * las que ya se han introducido correctamente.
 */
static void game_draw_input_progress(Direction *seq, int length, int current_index)
{
    int x = 10;
    int y = 10 + TEXT_LINE_HEIGHT * 3;  // misma línea que "TU INPUT:" pero desplazada

    // Limpiamos el área de entrada simple: un rectángulo horizontal
    // (esto es muy básico, optimizable según necesidades)
    ili9341_fill_rect(0, y - 2, ILI9341_WIDTH, TEXT_LINE_HEIGHT * 2, COLOR_BG);

    for (int i = 0; i < length; i++) {
        const char *s = direction_to_char(seq[i]);
        uint16_t color = (i < current_index) ? COLOR_GOOD : COLOR_TEXT;
        ili9341_draw_string(x, y, s, color, COLOR_BG, 2);
        x += 20;
    }
}

/**
 * Dibuja un temporizador simple en la parte inferior de la pantalla.
 * Muestra el tiempo restante en segundos (con resolución aproximada).
 */
static void game_draw_timer(uint32_t remaining_ms)
{
    char buf[32];
    float seconds = remaining_ms / 1000.0f;
    snprintf(buf, sizeof(buf), "TIEMPO: %.1fs", (double)seconds);

    int y = ILI9341_HEIGHT - TEXT_LINE_HEIGHT * 2;

    // Limpiar zona inferior
    ili9341_fill_rect(0, y - 2, ILI9341_WIDTH, TEXT_LINE_HEIGHT * 2 + 4, COLOR_BG);

    ili9341_draw_string(10, y, buf, COLOR_INFO, COLOR_BG, 2);
}

/**
 * Muestra el resultado del intento (ÉXITO o FALLO) en grande
 * en el centro de la pantalla.
 */
static void game_draw_result(int success)
{
    ili9341_fill_screen(COLOR_BG);

    const char *msg = success ? "EXITO" : "FALLO";
    uint16_t color = success ? COLOR_GOOD : COLOR_BAD;

    int x = 40;
    int y = ILI9341_HEIGHT / 2 - TEXT_LINE_HEIGHT;

    ili9341_draw_string(x, y, msg, color, COLOR_BG, 3);
}

/**
 * Convierte una dirección en un texto corto para dibujar en pantalla.
 * Aquí usamos:
 *  - DIR_UP    -> "U"
 *  - DIR_DOWN  -> "D"
 *  - DIR_LEFT  -> "L"
 *  - DIR_RIGHT -> "R"
 *
 * Puedes cambiarlo para dibujar flechas, iconos, etc.
 */
static const char *direction_to_char(Direction d)
{
    switch (d) {
    case DIR_UP:    return "U";
    case DIR_DOWN:  return "D";
    case DIR_LEFT:  return "L";
    case DIR_RIGHT: return "R";
    default:        return "?";
    }
}
