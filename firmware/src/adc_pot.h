#ifndef ADC_POT_H
#define ADC_POT_H

#include <avr/io.h>

void adc_init();

uint16_t read_pot_value();

#endif