#include "library_led_c.h"
#include "esp_err.h"

// Convierte porcentaje 0-100 a duty según la resolución elegida
static uint32_t percentage_to_duty(int percentage, ledc_timer_bit_t resolution)
{
    uint32_t max_duty = (1 << resolution) - 1;
    return (percentage * max_duty) / 100;
}

void config_led_rgb(led_rgb_t *led_rgb)
{
    // 1) Configurar el timer PWM
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = led_rgb->speed_mode,
        .duty_resolution  = led_rgb->duty_resolution,
        .timer_num        = led_rgb->timer,
        .freq_hz          = led_rgb->frequency,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // 2) Configurar canal rojo
    ledc_channel_config_t ch_red = {
        .speed_mode = led_rgb->speed_mode,
        .channel    = led_rgb->led_red.channel,
        .timer_sel  = led_rgb->timer,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = led_rgb->led_red.gpio_num,
        .duty       = 0,
        .hpoint     = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch_red));

    // 3) Configurar canal verde
    ledc_channel_config_t ch_green = {
        .speed_mode = led_rgb->speed_mode,
        .channel    = led_rgb->led_green.channel,
        .timer_sel  = led_rgb->timer,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = led_rgb->led_green.gpio_num,
        .duty       = 0,
        .hpoint     = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch_green));

    // 4) Configurar canal azul
    ledc_channel_config_t ch_blue = {
        .speed_mode = led_rgb->speed_mode,
        .channel    = led_rgb->led_blue.channel,
        .timer_sel  = led_rgb->timer,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = led_rgb->led_blue.gpio_num,
        .duty       = 0,
        .hpoint     = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ch_blue));
}

void set_led_rgb_percentage_given_values(led_rgb_t *led_rgb,
                                         int percentage_red,
                                         int percentage_green,
                                         int percentage_blue)
{
    uint32_t duty_red   = percentage_to_duty(percentage_red, led_rgb->duty_resolution);
    uint32_t duty_green = percentage_to_duty(percentage_green, led_rgb->duty_resolution);
    uint32_t duty_blue  = percentage_to_duty(percentage_blue, led_rgb->duty_resolution);

    ledc_set_duty(led_rgb->speed_mode, led_rgb->led_red.channel, duty_red);
    ledc_update_duty(led_rgb->speed_mode, led_rgb->led_red.channel);

    ledc_set_duty(led_rgb->speed_mode, led_rgb->led_green.channel, duty_green);
    ledc_update_duty(led_rgb->speed_mode, led_rgb->led_green.channel);

    ledc_set_duty(led_rgb->speed_mode, led_rgb->led_blue.channel, duty_blue);
    ledc_update_duty(led_rgb->speed_mode, led_rgb->led_blue.channel);
}