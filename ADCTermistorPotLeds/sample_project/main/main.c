#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "library_adc.h"
#include "library_rgb.h"
#include "library_button.h"


// Máquina de estados
typedef enum {
    CONFIG_RED = 0,
    CONFIG_BLUE,
    CONFIG_GREEN,
    SHOW_COLOR
} state_t;


// Variables globales
static state_t current_state = CONFIG_RED;

static int red_value = 0;
static int green_value = 0;
static int blue_value = 0;


// TAREA 1: Temperatura → RGB1
void task_temperature(void *pvParameters)
{
    while (1)
    {
        float temp = ntc_get_temperature();

        printf("Temperatura: %.2f °C\n", temp);

        if (temp < 25.0)
        {
            // Azul
            rgb_set_color(0, 0, 255, 1);
        }
        else if (temp >= 25.0 && temp <= 35.0)
        {
            // Verde
            rgb_set_color(0, 255, 0, 1);
        }
        else
        {
            // Rojo
            rgb_set_color(255, 0, 0, 1);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


// TAREA 2: Estados + Pot → RGB2
void task_states(void *pvParameters)
{
    while (1)
    {
        // Leer botón
        if (button_pressed())
        {
            current_state++;

            if (current_state > SHOW_COLOR)
            {
                current_state = CONFIG_RED;
            }
        }

        // Leer potenciómetro
        int pot_raw = adc_read_raw(ADC_CHANNEL_7);

        // Escalar a 0–255 (PWM)
        int value = (pot_raw * 255) / 4095;

        switch (current_state)
        {
            case CONFIG_RED:
                red_value = value;
                rgb_set_color(red_value, 0, 0, 2);
                break;

            case CONFIG_BLUE:
                blue_value = value;
                rgb_set_color(0, 0, blue_value, 2);
                break;

            case CONFIG_GREEN:
                green_value = value;
                rgb_set_color(0, green_value, 0, 2);
                break;

            case SHOW_COLOR:
                rgb_set_color(red_value, green_value, blue_value, 2);
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


// MAIN
void app_main(void)
{
    adc_init();
    rgb_init();
    button_init();

    xTaskCreate(task_temperature, "temp_task", 4096, NULL, 1, NULL);
    xTaskCreate(task_states, "state_task", 4096, NULL, 1, NULL);
}