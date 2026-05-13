#include "uart_commands.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "library_uart.h"
#include "library_adc.h"
#include "library_rgb.h"

#define CONFIG_RED    0
#define CONFIG_BLUE   1
#define CONFIG_GREEN  2
#define SHOW_COLOR    3

extern volatile int current_state;
extern volatile int red_value;
extern volatile int green_value;
extern volatile int blue_value;

static void trim_spaces(char *s)
{
    if (s == NULL) {
        return;
    }

    while (isspace((unsigned char)*s)) {
        memmove(s, s + 1, strlen(s));
    }

    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[len - 1] = '\0';
        len--;
    }
}

static void to_uppercase(char *s)
{
    if (s == NULL) {
        return;
    }

    for (; *s; ++s) {
        *s = (char)toupper((unsigned char)*s);
    }
}

static int percent_from_pwm(int value)
{
    if (value < 0) value = 0;
    if (value > 255) value = 255;
    return (value * 100) / 255;
}

static void print_rgb_status(void)
{
    if (current_state == CONFIG_RED) {
        uart_comm_write_fmt("STATE=RED, RED=%d%% (raw=%d)\r\n",
                            percent_from_pwm(red_value), red_value);
    } else if (current_state == CONFIG_BLUE) {
        uart_comm_write_fmt("STATE=BLUE, BLUE=%d%% (raw=%d)\r\n",
                            percent_from_pwm(blue_value), blue_value);
    } else if (current_state == CONFIG_GREEN) {
        uart_comm_write_fmt("STATE=GREEN, GREEN=%d%% (raw=%d)\r\n",
                            percent_from_pwm(green_value), green_value);
    } else {
        uart_comm_write_fmt("STATE=SHOW_COLOR, RED=%d%% (raw=%d), GREEN=%d%% (raw=%d), BLUE=%d%% (raw=%d)\r\n",
                            percent_from_pwm(red_value), red_value,
                            percent_from_pwm(green_value), green_value,
                            percent_from_pwm(blue_value), blue_value);
    }
}

static void print_single_color(const char *color_name, int value)
{
    uart_comm_write_fmt("%s=%d%% (raw=%d)\r\n", color_name, percent_from_pwm(value), value);
}

static void handle_get_limit(const char *key)
{
    if (strcmp(key, "BLUE_LED_VALUE_MIN") == 0) {
        uart_comm_write_fmt("BLUE_LED_VALUE_MIN=%.2f\r\n", temp_limits_get_blue_min());
    } else if (strcmp(key, "BLUE_LED_VALUE_MAX") == 0) {
        uart_comm_write_fmt("BLUE_LED_VALUE_MAX=%.2f\r\n", temp_limits_get_blue_max());
    } else if (strcmp(key, "GREEN_LED_VALUE_MIN") == 0) {
        uart_comm_write_fmt("GREEN_LED_VALUE_MIN=%.2f\r\n", temp_limits_get_green_min());
    } else if (strcmp(key, "GREEN_LED_VALUE_MAX") == 0) {
        uart_comm_write_fmt("GREEN_LED_VALUE_MAX=%.2f\r\n", temp_limits_get_green_max());
    } else if (strcmp(key, "RED_LED_VALUE_MIN") == 0) {
        uart_comm_write_fmt("RED_LED_VALUE_MIN=%.2f\r\n", temp_limits_get_red_min());
    } else if (strcmp(key, "RED_LED_VALUE_MAX") == 0) {
        uart_comm_write_fmt("RED_LED_VALUE_MAX=%.2f\r\n", temp_limits_get_red_max());
    } else {
        uart_comm_write_str("ERR: limite no reconocido\r\n");
    }
}

static void handle_set_limit(const char *key, float value)
{
    if (strcmp(key, "BLUE_LED_VALUE_MIN") == 0) {
        temp_limits_set_blue_min(value);
        uart_comm_write_fmt("OK BLUE_LED_VALUE_MIN=%.2f\r\n", value);
    } else if (strcmp(key, "BLUE_LED_VALUE_MAX") == 0) {
        temp_limits_set_blue_max(value);
        uart_comm_write_fmt("OK BLUE_LED_VALUE_MAX=%.2f\r\n", value);
    } else if (strcmp(key, "GREEN_LED_VALUE_MIN") == 0) {
        temp_limits_set_green_min(value);
        uart_comm_write_fmt("OK GREEN_LED_VALUE_MIN=%.2f\r\n", value);
    } else if (strcmp(key, "GREEN_LED_VALUE_MAX") == 0) {
        temp_limits_set_green_max(value);
        uart_comm_write_fmt("OK GREEN_LED_VALUE_MAX=%.2f\r\n", value);
    } else if (strcmp(key, "RED_LED_VALUE_MIN") == 0) {
        temp_limits_set_red_min(value);
        uart_comm_write_fmt("OK RED_LED_VALUE_MIN=%.2f\r\n", value);
    } else if (strcmp(key, "RED_LED_VALUE_MAX") == 0) {
        temp_limits_set_red_max(value);
        uart_comm_write_fmt("OK RED_LED_VALUE_MAX=%.2f\r\n", value);
    } else {
        uart_comm_write_str("ERR: limite no reconocido\r\n");
    }
}

void uart_commands_process_line(const char *line)
{
    if (line == NULL) {
        return;
    }

    char cmd[128];
    strncpy(cmd, line, sizeof(cmd) - 1);
    cmd[sizeof(cmd) - 1] = '\0';

    trim_spaces(cmd);
    if (strlen(cmd) == 0) {
        return;
    }

    to_uppercase(cmd);

    if (strcmp(cmd, "TEMP?") == 0) {
        uart_comm_write_fmt("TEMP=%.2f C\r\n", ntc_get_temperature());
        return;
    }

    if (strcmp(cmd, "RGB?") == 0) {
        print_rgb_status();
        return;
    }

    if (strcmp(cmd, "GET RED") == 0) {
        print_single_color("RED", red_value);
        return;
    }

    if (strcmp(cmd, "GET GREEN") == 0) {
        print_single_color("GREEN", green_value);
        return;
    }

    if (strcmp(cmd, "GET BLUE") == 0) {
        print_single_color("BLUE", blue_value);
        return;
    }

    if (strcmp(cmd, "GET STATE") == 0) {
        if (current_state == CONFIG_RED) {
            uart_comm_write_str("STATE=RED\r\n");
        } else if (current_state == CONFIG_BLUE) {
            uart_comm_write_str("STATE=BLUE\r\n");
        } else if (current_state == CONFIG_GREEN) {
            uart_comm_write_str("STATE=GREEN\r\n");
        } else {
            uart_comm_write_str("STATE=SHOW_COLOR\r\n");
        }
        return;
    }

    if (strncmp(cmd, "GET ", 4) == 0) {
        char key[64];
        if (sscanf(cmd, "GET %63s", key) == 1) {
            handle_get_limit(key);
            return;
        }
    }

    if (strncmp(cmd, "SET ", 4) == 0) {
        char key[64];
        float value;
        if (sscanf(cmd, "SET %63s %f", key, &value) == 2) {
            handle_set_limit(key, value);
            return;
        }
        uart_comm_write_str("ERR: formato SET invalido\r\n");
        return;
    }

    uart_comm_write_str("ERR: comando no valido\r\n");
    uart_comm_write_fmt("RECIBI: %s\r\n", line);
}