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

inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint16_t read_pot_value() {
    uint16_t pot_value = read_adc();
    return map(pot_value, 0, 1023, 0, 120);    
}