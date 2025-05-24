#include <avr/io.h>

#define BUZZER_BIT  PB2
#define BUZZER_DDR  DDRB
#define BUZZER_PORT PORTB

void buzzer_init();

void play_note(uint16_t frequency);

void stop_note();