#include "library_adc.h"
#include <math.h>

static adc_oneshot_unit_handle_t adc_handle;

// CONFIGURACIÓN
#define ADC_CHANNEL_THERMISTOR ADC_CHANNEL_6 // GPIO34
#define ADC_CHANNEL_POT        ADC_CHANNEL_7 // GPIO35

#define VCC 3.3
#define ADC_MAX 4095.0

// NTC
#define R_FIXED 10000.0 // Resistencia fija en serie con el termistor (10kΩ)
#define BETA 3950.0 // Coeficiente Beta del termistor (depende del modelo, este es un valor común para NTC de 10kΩ)
#define T0 298.15 // Temperatura de referencia en Kelvin (25°C = 298.15K)
#define R0 10000.0 // Resistencia del termistor a la temperatura de referencia (10kΩ a 25°C)


void adc_init(void)
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1, // Usamos ADC1
    };

    adc_oneshot_new_unit(&init_config, &adc_handle); 

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, //Resolución máxima soportada por el ADC (12 bits -> 4095)
        .atten = ADC_ATTEN_DB_12, // Atenuación de 12 dB para medir hasta ~3.3V
    };

    adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_THERMISTOR, &config); // Configura el canal del termistor
    adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_POT, &config); // Configura el canal del potenciómetro
}

// Inicializa ADC1
// Configura 2 canales:
// termistor
// potenciómetro

int adc_read_raw(adc_channel_t channel)
{
    int raw = 0;
    adc_oneshot_read(adc_handle, channel, &raw);
    return raw;
}

// Lee el valor digital directamente del ADC para el canal especificado

float adc_read_voltage(adc_channel_t channel)
{
    int raw = adc_read_raw(channel);
    return (raw / ADC_MAX) * VCC; 
}

// Convierte el valor digital del ADC a voltaje.

float ntc_get_temperature(void)
{
    float V = adc_read_voltage(ADC_CHANNEL_THERMISTOR);

    float R_ntc = R_FIXED * (V / (VCC - V));

    float tempK = 1.0 / ((1.0 / T0) + (1.0 / BETA) * log(R_ntc / R0));

    float tempC = tempK - 273.15;

    return tempC;
}

// Lee el voltaje del termistor, calcula su resistencia y luego la temperatura usando la fórmula de Steinhart-Hart.