#ifndef LIBRARY_ADC_H
#define LIBRARY_ADC_H

#include "esp_adc/adc_oneshot.h"

void adc_init(void);
int adc_read_raw(adc_channel_t channel);
float adc_read_voltage(adc_channel_t channel);
float ntc_get_temperature(void);

#endif