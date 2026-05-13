#include "library_rgb.h"
#include "driver/ledc.h"

// RGB1 (temperatura)
#define R1 25
#define G1 26
#define B1 27

// RGB2 (manual)
#define R2 14
#define G2 12
#define B2 13


// Variables para guardar el estado actual de cada LED RGB
static int rgb1_r = 0;
static int rgb1_g = 0;
static int rgb1_b = 0;

static int rgb2_r = 0;
static int rgb2_g = 0;
static int rgb2_b = 0;


void rgb_init(void)
{
    ledc_timer_config_t timer = {
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 5000,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0
    };

    ledc_timer_config(&timer);

    int pins[6] = {R1, G1, B1, R2, G2, B2};

    for (int i = 0; i < 6; i++) {
        ledc_channel_config_t ch = {
            .channel = i,
            .duty = 0,
            .gpio_num = pins[i],
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .timer_sel = LEDC_TIMER_0
        };
        ledc_channel_config(&ch);
    }
}


void rgb_set_color(int r, int g, int b, int led)
{
    int offset = (led == 1) ? 0 : 3;

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, offset + 0, r);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, offset + 0);

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, offset + 1, g);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, offset + 1);

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, offset + 2, b);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, offset + 2);

    // Guardar valores actuales
    if (led == 1)
    {
        rgb1_r = r;
        rgb1_g = g;
        rgb1_b = b;
    }
    else
    {
        rgb2_r = r;
        rgb2_g = g;
        rgb2_b = b;
    }
}

void rgb_get_color(int led, int *r, int *g, int *b)
{
    if (led == 1)
    {
        if (r != NULL) *r = rgb1_r;
        if (g != NULL) *g = rgb1_g;
        if (b != NULL) *b = rgb1_b;
    }
    else
    {
        if (r != NULL) *r = rgb2_r;
        if (g != NULL) *g = rgb2_g;
        if (b != NULL) *b = rgb2_b;
    }
}
// Configura los temporizadores y canales para controlar dos LEDs RGB usando PWM.
// El primer LED (RGB1) se usará para mostrar la temperatura, y el segundo (RGB2) para control manual.
// La función rgb_set_color permite establecer el color de cada LED RGB individualmente, especificando los valores de rojo, verde y azul (0-255) para el LED seleccionado.