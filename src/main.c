#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/touch_pad.h"
#include "esp_log.h"
#include "esp_timer.h" // Para obtener microsegundos

static const char *TAG = "HELLDIVER_OS";

// Configuración
#define TOUCH_PAD_PIN    TOUCH_PAD_NUM3      // GPIO 15
#define TOUCH_THRESHOLD  400     // Umbral de detección
#define SEQUENCE_TIMEOUT 2000000 // 2 segundos en microsegundos

int combatDiary();
int touchPractice();

typedef enum {
    IDLE,
    STEP_1,
    STEP_2,
    SUCCESS
} state_t;

void app_main(void) {

    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG, "Iniciando sistema de Estratagemas...");
    combatDiary();
    
}

int combatDiary(){
    /*1. El "Diario de Combate" (Persistencia con NVS)

    El ESP32 tiene una partición de memoria flash llamada NVS (Non-Volatile Storage). Es un sistema de archivos clave-valor.

    El Reto: Crea un programa que cuente cuántas veces se ha reiniciado el dispositivo y guarde un "historial de misiones" (strings).

    Concepto CS: Gestión de memoria flash, manejo de estructuras en C y uso de la librería nvs_flash.h.

    Uso Milsim: Registrar cuántas veces ha sido "activada" una bomba prop o guardar el ID del jugador que la desarmó sin perder datos al apagarla. */
    return 0;
}

int touchPractice(){
    // Inicializar Touch
    touch_pad_init();
    touch_pad_config(TOUCH_PAD_PIN, 0);

    state_t current_state = IDLE;
    int64_t last_touch_time = 0;

    while (1) {
        uint16_t val;
        touch_pad_read(TOUCH_PAD_PIN, &val);
        int64_t now = esp_timer_get_time();

        // Lógica de Timeout: Si pasa mucho tiempo, vuelve a IDLE
        if (current_state != IDLE && (now - last_touch_time) > SEQUENCE_TIMEOUT) {
            ESP_LOGW(TAG, "Timeout: Secuencia reiniciada");
            current_state = IDLE;
        }

        // Detección de toque (flanco de bajada manual)
        if (val < TOUCH_THRESHOLD) {
            last_touch_time = now;

            switch (current_state) {
                case IDLE:
                    current_state = STEP_1;
                    ESP_LOGI(TAG, "Combo: [X] [ ] [ ]");
                    break;
                case STEP_1:
                    current_state = STEP_2;
                    ESP_LOGI(TAG, "Combo: [X] [X] [ ]");
                    break;
                case STEP_2:
                    current_state = SUCCESS;
                    ESP_LOGI(TAG, "Combo: [X] [X] [X]");
                    break;
                default: break;
            }

            if (current_state == SUCCESS) {
                ESP_LOGW(TAG, "¡ESTRATAGEMA DESPLEGADA: REFUERZOS!");
                current_state = IDLE; // Reiniciar tras éxito
            }

            // "Debounce" rudimentario: esperar a que sueltes el pin
            while (val < TOUCH_THRESHOLD) {
                touch_pad_read(TOUCH_PAD_PIN, &val);
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Muestreo cada 50ms
    }
}