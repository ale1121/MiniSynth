#ifndef ADC_POT_H
#define ADC_POT_H

#include <avr/io.h>

void adc_init();

float read_pot_value();

#endif