#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <avr/interrupt.h>

void timer0_init();

uint32_t _millis();

void _delay(uint32_t ms);

#endif