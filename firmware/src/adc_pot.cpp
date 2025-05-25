#include "adc_pot.h"

void adc_init() {
    // voltage reference 01 - AVCC with external capacitor on AREF pin
    // input channel 0000 - ADC0
    ADMUX = (1 << REFS0);

    // ADC presclaler 111 - 128
    ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // enable ADC
    ADCSRA |= (1 << ADEN);
}

uint16_t read_adc() {
    // start conversion
    ADCSRA |= (1 << ADSC);

    // wait for conversion to complete
    while (ADCSRA & (1 << ADSC));

    return ADC;
}

float read_pot_value() {
    uint16_t pot_value = read_adc();

    // map to 0.0 - 12.0
    return (pot_value * 12.0) / 1023.0;   
}