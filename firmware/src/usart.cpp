#include "usart.h"

void usart0_init(uint16_t ubrr) {
    // set baud rate
    UBRR0H = (ubrr >> 8);
    UBRR0L = ubrr;

    // enable RX and TX
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    // frame format: 8 data bits, 1 stop bit, no parity
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}


void usart0_transmit(unsigned char c) {
    // wait for empty buffer
    while (!(UCSR0A & (1 << UDRE0)));

    // send character
    UDR0 = c;
}


void usart0_print(const char *str) {
    while (*str) {
        usart0_transmit(*str++);
    }
}
