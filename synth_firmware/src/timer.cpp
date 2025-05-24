#include "timer.h"

volatile uint32_t timer0_ms = 0;

ISR(TIMER0_COMPA_vect) {
    timer0_ms++;
}

void timer0_init() {
    // mode 010 - CTC, TOP = OCR0A
    TCCR0A = (1 << WGM01);

    // clock select 011 - prescaler 64
    TCCR0B = (1 << CS01) | (1 << CS00);

    // OCR0A = F_CPU / (PS * tc) - 1
    //       = 16MHz / (64 * 1kHz) - 1
    //       = 249 ticks/ms
    OCR0A = 249;

    // compare match A interrupt
    TIMSK0 = (1 << OCIE0A);
}

uint32_t _millis() {
    uint32_t ms;
    cli();
    ms = timer0_ms;
    sei();
    return ms;
}

void _delay(uint32_t ms) {
    uint32_t start = _millis();
    while (_millis() - start < ms);
}