#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "library_adc.h"
#include "library_rgb.h"
#include "library_button.h"
#include "library_uart.h"
#include "uart_commands.h"

// Máquina de estados
typedef enum {
    CONFIG_RED = 0,
    CONFIG_BLUE,
    CONFIG_GREEN,
    SHOW_COLOR
} state_t;

// Variables globales para que UART las pueda consultar
volatile int current_state = CONFIG_RED;
volatile int red_value = 0;
volatile int green_value = 0;
volatile int blue_value = 0;

#define UART_LINE_MAX 128

// TAREA 1: Temperatura → RGB1
void task_temperature(void *pvParameters)
{
    while (1)
    {
        float temp = ntc_get_temperature();

        float blue_min  = temp_limits_get_blue_min();
        float blue_max  = temp_limits_get_blue_max();
        float green_min = temp_limits_get_green_min();
        float green_max = temp_limits_get_green_max();
        float red_min   = temp_limits_get_red_min();
        float red_max   = temp_limits_get_red_max();

        if (temp >= blue_min && temp <= blue_max)
        {
            rgb_set_color(0, 0, 255, 1);
        }
        else if (temp >= green_min && temp <= green_max)
        {
            rgb_set_color(0, 255, 0, 1);
        }
        else if (temp >= red_min && temp <= red_max)
        {
            rgb_set_color(255, 0, 0, 1);
        }
        else
        {
            rgb_set_color(0, 0, 0, 1);
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// TAREA 2: Estados + Pot → RGB2
void task_states(void *pvParameters)
{
    while (1)
    {
        if (button_pressed())
        {
            current_state++;

            if (current_state > SHOW_COLOR)
            {
                current_state = CONFIG_RED;
            }
        }

        int pot_raw = adc_read_raw(ADC_CHANNEL_7);
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

// TAREA UART: recibe una línea, la procesa y responde
void task_uart(void *pvParameters)
{
    char line[UART_LINE_MAX];
    int index = 0;
    uint8_t ch;

    uart_comm_write_str("\r\nUART lista.\r\n");
    uart_comm_write_str("Comandos: TEMP?, RGB?, GET <limite>, SET <limite> <valor>\r\n");

    while (1)
    {
        // Leemos de a 1 byte
        int len = uart_comm_read_bytes(&ch, 1, pdMS_TO_TICKS(20));

        if (len > 0)
        {
            // --- NUEVO: ECO LOCAL ---
            // Reenviamos lo que recibimos para poder verlo en la terminal
            uart_write_bytes(UART_COMM_PORT, &ch, 1);

            // Manejo de fin de línea (Enter)
            // Aceptamos tanto \r (Windows/Classic) como \n (Unix/Linux)
            if (ch == '\r' || ch == '\n')
            {
                line[index] = '\0'; // Cerramos el string
                
                if (index > 0)
                {
                    uart_comm_write_str("\r\n"); // Salto de línea estético en la terminal
                    uart_commands_process_line(line);
                }
                index = 0; // Reiniciamos buffer
            }
            else if (ch == 0x08 || ch == 0x7F) // Manejo de "Backspace" (Borrar)
            {
                if (index > 0) index--;
            }
            else
            {
                if (index < UART_LINE_MAX - 1)
                {
                    line[index++] = (char)ch;
                }
                else
                {
                    index = 0;
                    uart_comm_write_str("\r\nERR: linea demasiado larga\r\n");
                }
            }
        }
    }
}

// MAIN
void app_main(void)
{
    adc_init();
    rgb_init();
    button_init();
    uart_comm_init();

    xTaskCreate(task_temperature, "temp_task", 4096, NULL, 1, NULL);
    xTaskCreate(task_states, "state_task", 4096, NULL, 1, NULL);
    xTaskCreate(task_uart, "uart_task", 4096, NULL, 2, NULL);
}