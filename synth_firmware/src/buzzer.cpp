#ifndef BUZZER_H
#define BUZZER_H

#include "buzzer.h"


void buzzer_init() {
    // set pin as output
    BUZZER_DDR |= (1 << BUZZER_BIT);

    // set up timer 1

    // clear OC1B on compare match
    TCCR1A = (1 << COM1B1);

    // mode 1110 - Fast PWM, TOP = ICR1
    TCCR1A |= (1 << WGM11);  
    TCCR1B = (1 << WGM13) | (1 << WGM12);
    
    // clock select 001 - no prescaler
    TCCR1B |= (1 << CS10);
}

void play_note(uint16_t frequency) {
    // TOP = F_CPU / (PS * frequency) - 1
    uint32_t top = (F_CPU / frequency) - 1;

    ICR1 = top;

    // 50% duty cycle
    OCR1B = top / 2;

    // set pin as output
    BUZZER_DDR |= (1 << BUZZER_BIT);

    // clear OC1B on compare match
    TCCR1A |= (1 << COM1B1);
}

void stop_note() {
    // disconnect OC1B
    TCCR1A &= ~(1 << COM1B1);

    // set pin low
    BUZZER_PORT &= ~(1 << BUZZER_BIT);

    // set pin as input
    BUZZER_DDR &= ~(1 << BUZZER_BIT);
}

#endif