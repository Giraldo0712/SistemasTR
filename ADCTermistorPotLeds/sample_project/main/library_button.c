#include "library_button.h"
#include "driver/gpio.h"

#define BUTTON_PIN 4

void button_init(void)
{
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);
}

int button_pressed(void)
{
    static int last = 1;
    int current = gpio_get_level(BUTTON_PIN);

    if (last == 1 && current == 0) {
        last = current;
        return 1;
    }

    last = current;
    return 0;
}

// Configura el pin del botón como entrada con resistencia pull-up interna.
// La función button_pressed devuelve 1 solo en el momento en que se detecta una transición