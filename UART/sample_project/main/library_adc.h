#ifndef LIBRARY_ADC_H
#define LIBRARY_ADC_H

#include "esp_adc/adc_oneshot.h"

void adc_init(void);
int adc_read_raw(adc_channel_t channel);
float adc_read_voltage(adc_channel_t channel);
float ntc_get_temperature(void);


// ===== SETTERS =====
void temp_limits_set_blue_min(float value);
void temp_limits_set_blue_max(float value);

void temp_limits_set_green_min(float value);
void temp_limits_set_green_max(float value);

void temp_limits_set_red_min(float value);
void temp_limits_set_red_max(float value);


// ===== GETTERS =====
float temp_limits_get_blue_min(void);
float temp_limits_get_blue_max(void);

float temp_limits_get_green_min(void);
float temp_limits_get_green_max(void);

float temp_limits_get_red_min(void);
float temp_limits_get_red_max(void);

#endif