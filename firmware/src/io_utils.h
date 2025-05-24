#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <avr/io.h>

struct IOPin {
    volatile uint8_t* pin;
    volatile uint8_t* ddr;
    volatile uint8_t* port;
    uint8_t bit;
};

inline void conf_input(IOPin p) {
    *(p.ddr) &= ~(1 << p.bit); 
}

inline void conf_input_pullup(IOPin p) {
    *(p.ddr) &= ~(1 << p.bit); 
    *(p.port) |= (1 << p.bit);
}

inline void conf_output(IOPin p) {
    *(p.ddr) |= (1 << p.bit);
}

inline uint8_t read_pin(IOPin p) {
    return (*(p.pin) & (1 << p.bit)) != 0; 
}

inline void set_pin_low(IOPin p) {
    *(p.port) &= ~(1 << p.bit);
}

inline void set_pin_high(IOPin p) {
    (*p.port) |= (1 << p.bit);
}

#endif