#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "library_led_c.h"

#define BTN_R GPIO_NUM_18
#define BTN_G GPIO_NUM_19
#define BTN_B GPIO_NUM_21

#define STEP_PERCENT 10

QueueHandle_t gpio_evt_queue = NULL;

/* Prototipo de la ISR */
static void IRAM_ATTR gpio_isr_handler(void *arg);

static void config_buttons(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BTN_R) | (1ULL << BTN_G) | (1ULL << BTN_B),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };

    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BTN_R, gpio_isr_handler, (void *)BTN_R);
    gpio_isr_handler_add(BTN_G, gpio_isr_handler, (void *)BTN_G);
    gpio_isr_handler_add(BTN_B, gpio_isr_handler, (void *)BTN_B);
}

static void print_rgb_state(int r, int g, int b)
{
    printf("R: %d%%, G: %d%%, B: %d%%\r\n", r, g, b);
}

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    int pin = (int)arg;
    xQueueSendFromISR(gpio_evt_queue, &pin, NULL);
}

void rgb_task(void *pvParameters)
{
    gpio_evt_queue = xQueueCreate(10, sizeof(int));

    config_buttons();

    led_rgb_t led_rgb = {
        .led_red = {
            .gpio_num = GPIO_NUM_25,
            .channel = LEDC_CHANNEL_0
        },
        .led_green = {
            .gpio_num = GPIO_NUM_26,
            .channel = LEDC_CHANNEL_1
        },
        .led_blue = {
            .gpio_num = GPIO_NUM_27,
            .channel = LEDC_CHANNEL_2
        },
        .timer = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .frequency = 5000,
        .speed_mode = LEDC_LOW_SPEED_MODE
    };

    config_led_rgb(&led_rgb);

    int r = 0, g = 0, b = 0;
    int pin;

    while (1)
    {
        if (xQueueReceive(gpio_evt_queue, &pin, portMAX_DELAY))
        {
            vTaskDelay(pdMS_TO_TICKS(50));

            if (gpio_get_level(pin) == 0)
            {
                if (pin == BTN_R) {
                    r += STEP_PERCENT;
                    if (r > 100) r = 100;
                } else if (pin == BTN_G) {
                    g += STEP_PERCENT;
                    if (g > 100) g = 100;
                } else if (pin == BTN_B) {
                    b += STEP_PERCENT;
                    if (b > 100) b = 100;
                }

                set_led_rgb_percentage_given_values(&led_rgb, r, g, b);
                print_rgb_state(r, g, b);
            }
        }
    }
}

void app_main(void)
{
    xTaskCreate(
        rgb_task,
        "rgb_task",
        4096,
        NULL,
        5,
        NULL
    );
}