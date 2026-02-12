/**
 * Driver mínimo y didáctico para ILI9341 en ESP32 (ESP-IDF)
 * --------------------------------------------------------
 *
 * OBJETIVO:
 *   - Mostrar cómo inicializar el controlador ILI9341 por SPI.
 *   - Ofrecer primitivas muy sencillas: limpiar pantalla, dibujar
 *     píxeles, rectángulos y texto con una fuente bitmap básica.
 *
 * COSAS A TENER EN CUENTA:
 *   - Este driver está simplificado para ser entendible, no optimizado.
 *   - Usa SPI en modo maestro (HSPI / VSPI) del ESP32.
 *   - Trabaja en formato de color RGB565 (16 bits por píxel).
 *
 * ADÁPTALO A TU HARDWARE:
 *   - Revisa los #define de pines en ili9341.c (CS, DC, RST, BL, etc.).
 *   - Asegúrate de que la alimentación y el backlight de la pantalla
 *     están correctamente conectados.
 */

#pragma once

#include <stdint.h>

// Dimensiones típicas del ILI9341 en orientación vertical (portrait)
// Muchas pantallas son 240x320; aquí usaremos 240x320 como ejemplo.

#define ILI9341_WIDTH   240
#define ILI9341_HEIGHT  320

// Colores básicos en formato RGB565 (5 bits R, 6 bits G, 5 bits B)
// Cada componente se expresa en el rango 0-31 (R y B) o 0-63 (G).

#define ILI9341_RGB565(r, g, b) \
    (uint16_t)((((r) & 0x1F) << 11) | (((g) & 0x3F) << 5) | ((b) & 0x1F))

// Paleta corta de colores útiles para el juego

#define ILI9341_COLOR_BLACK   ILI9341_RGB565(0,   0,   0)
#define ILI9341_COLOR_WHITE   ILI9341_RGB565(31, 63, 31)
#define ILI9341_COLOR_RED     ILI9341_RGB565(31, 0,  0)
#define ILI9341_COLOR_GREEN   ILI9341_RGB565(0,  63, 0)
#define ILI9341_COLOR_BLUE    ILI9341_RGB565(0,  0,  31)
#define ILI9341_COLOR_YELLOW  ILI9341_RGB565(31, 63, 0)
#define ILI9341_COLOR_CYAN    ILI9341_RGB565(0,  63, 31)
#define ILI9341_COLOR_MAGENTA ILI9341_RGB565(31, 0,  31)

// ------------------------------------------------------------
// API PÚBLICA DEL DRIVER
// ------------------------------------------------------------

/**
 * Inicializa el bus SPI y el controlador ILI9341.
 * - Configura los pines de control (CS, DC, RST, BL).
 * - Envía la secuencia básica de inicialización al ILI9341.
 */
void ili9341_init(void);

/**
 * Rellena toda la pantalla con un único color.
 */
void ili9341_fill_screen(uint16_t color);

/**
 * Dibuja un único píxel en (x, y) con el color indicado.
 * Coordenadas: 0 <= x < ILI9341_WIDTH, 0 <= y < ILI9341_HEIGHT.
 */
void ili9341_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

/**
 * Rellena un rectángulo (x, y, w, h) con el color indicado.
 *
 *  - x, y: esquina superior izquierda.
 *  - w, h: ancho y alto en píxeles.
 */
void ili9341_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * Dibuja un carácter ASCII sencillo (fuente 5x7) escalado.
 *
 *  - x, y: posición de la esquina superior izquierda del carácter.
 *  - ch:   carácter ASCII (solo se soporta un subconjunto básico).
 *  - color: color de primer plano.
 *  - bg:    color de fondo (se repinta el bloque completo).
 *  - scale: factor de escala entero (1 = 5x7, 2 = 10x14, etc.).
 */
void ili9341_draw_char(uint16_t x, uint16_t y, char ch,
                       uint16_t color, uint16_t bg, uint8_t scale);

/**
 * Dibuja una cadena de texto usando la fuente 5x7.
 *
 * Cada carácter se dibuja con ili9341_draw_char, avanzando en x.
 */
void ili9341_draw_string(uint16_t x, uint16_t y, const char *text,
                         uint16_t color, uint16_t bg, uint8_t scale);
