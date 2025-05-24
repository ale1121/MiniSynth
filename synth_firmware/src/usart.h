#ifndef USART_H
#define USART_H

#include <avr/io.h>

#define BAUD 9600
#define UBBR_VAL ((F_CPU / (16UL * BAUD)) - 1)

void usart0_init(uint16_t ubrr);

void usart0_print(const char *str);

#endif