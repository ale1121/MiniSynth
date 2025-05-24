#include <stdio.h>
#include <math.h>
#include "io_utils.h"
#include "buzzer.h"
#include "adc_pot.h"
#include "timer.h"
#include "usart.h"


const uint16_t note_freqs[8] = {262, 294, 330, 349, 392, 440, 494, 523};
const unsigned long debounce_delay = 30;


IOPin note_btns[8] = {
    {&PIND, &DDRD, &PORTD, PD2},
    {&PIND, &DDRD, &PORTD, PD3},
    {&PIND, &DDRD, &PORTD, PD4},
    {&PIND, &DDRD, &PORTD, PD5},
    {&PIND, &DDRD, &PORTD, PD6},
    {&PIND, &DDRD, &PORTD, PD7},
    {&PINB, &DDRB, &PORTB, PB0},
    {&PINB, &DDRB, &PORTB, PB1}
};

IOPin rec_btn =  {&PINC, &DDRC, &PORTC, PC1};
IOPin bt_state = {&PINB, &DDRB, &PORTB, PB5};
IOPin bt_led =   {&PINC, &DDRC, &PORTC, PC2};
IOPin rec_led =  {&PINC, &DDRC, &PORTC, PC3};


int current_note = -1;
int prev_note = -1;
unsigned long last_note_debounce_time = 0;

bool record_mode = false;
int prev_rec_btn_state = 1;
unsigned long last_rec_debounce_time = 0;

bool bt_connected = false;


// send "note on" message: "1 <timestamp> <frequency>"
void send_note_on(unsigned long timestamp, uint16_t freq) {
	char buffer[20];
	sprintf(buffer, "1 %lu %hu\n", _millis(), freq);
	usart0_print(buffer);
}


// send "note off" message: "0 <timestamp> 0"
void send_note_off(unsigned long timestamp) {
	char buffer[20];
	sprintf(buffer, "0 %lu 0\n", _millis());
	usart0_print(buffer);
}


// blink bluetooth led twice
void bt_not_connected_warning() {
	for (int i = 0; i < 2; i++) {
        set_pin_high(bt_led);
		_delay(100);
        set_pin_low(bt_led);
		_delay(100);
	}
}


void update_notes() {
	int note = -1;
	
	// poll button pins
	for(int i = 0; i < 8; i++) {
		if (read_pin(note_btns[i]) == 0) {
			note = i;
			break;
		}
	}

	// debounce
	if (note != prev_note) {
		last_note_debounce_time = _millis();
		prev_note = note;
	}

	if ((_millis() - last_note_debounce_time) > debounce_delay && note != current_note) {
		// note change

		if (note == -1) {
			// no note playing, stop buzzer
			stop_note();

			// send message if recording
			if (record_mode) send_note_off(_millis());

		} else {
			// play selected note

			// adjust pitch from potentiometer value
			float pitch_shift = read_pot_value() / 10.0;
			uint16_t freq = note_freqs[note] * pow(2, pitch_shift / 12.0);

			play_note(freq);

			// send message if recording
			if (record_mode) send_note_on(_millis(), freq);
		}
		
		// update current note
		current_note = note;
	}
}


void update_rec_mode() {
	uint8_t rec_btn_state = read_pin(rec_btn);

	if (rec_btn_state == 0 && prev_rec_btn_state == 1) {
		// button pressed, toggle record mode

		if (!record_mode) {
			if (!bt_connected) {
				// bluetooth not connected, can't enable record mode
				bt_not_connected_warning();
			} else {
				// enable record mode
				record_mode = true;
                set_pin_high(rec_led);
				usart0_print("RECORD_ON\n");
			}
		} else {
			// disable record mode
			record_mode = false;
            set_pin_low(rec_led);
			usart0_print("RECORD_OFF\n");
		}

		// debounce
		while (read_pin(rec_btn) == 0);
		_delay(debounce_delay);
	}

	prev_rec_btn_state = rec_btn_state;
}


void update_bt_state() {
    bt_connected = read_pin(bt_state);

	// update bluetooth led
    if (bt_connected) {
        set_pin_high(bt_led);
    } else {
        set_pin_low(bt_led);
    }

	if (!bt_connected && record_mode) {
		// lost connection, disable record mode
        set_pin_low(rec_led);
		record_mode = false;
		bt_not_connected_warning();
	}
}


void setup() {
	cli();

	usart0_init(UBBR_VAL);

	for (int i = 0; i < 8; i++) {
        conf_input_pullup(note_btns[i]);
	}

    conf_input_pullup(rec_btn);
    conf_input(bt_state);
    conf_output(bt_led);
    conf_output(rec_led);
	stop_note();
    adc_init();

	buzzer_init();
	timer0_init();
	sei();
}


int main(void) {
	setup();
	while(1) {
		update_bt_state();
		update_notes();
		update_rec_mode();
	}
}
