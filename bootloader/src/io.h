#ifndef _IO_H_
#define _IO_H_

/* Raw input-output operation of bootloader of MTB-UNI v4 module.
 */

#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>

#define PIN_LED_RED PD7
#define PIN_LED_GREEN PB0
#define PIN_LED_BLUE PB1

#define PIN_BUTTON PD5

#define PIN_UART_RX PD0
#define PIN_UART_TX PD1
#define PIN_UART_DIR PD3

static inline void io_init(void) {
	DDRB = (1 << PIN_LED_GREEN) | (1 << PIN_LED_BLUE);
	DDRC = 0;
	DDRD = (1 << PIN_LED_RED);
	DDRE = 0;
	PORTB = 0;
	PORTC = 0;
	PORTD = (1 << PIN_BUTTON) | (1 << PIN_UART_RX); // pull-up
	PORTE = 0;
	// DDRD |= (1 << PIN_UART_DIR) | (1 << PIN_UART_TX); // TODO is it required?
}

static inline void io_led_red_on(void) { PORTD |= (1 << PIN_LED_RED); }
static inline void io_led_red_off(void) { PORTD &= ~(1 << PIN_LED_RED); }
static inline void io_led_green_on(void) { PORTB |= (1 << PIN_LED_GREEN); }
static inline void io_led_green_off(void) { PORTB &= ~(1 << PIN_LED_GREEN); }
static inline void io_led_blue_on(void) { PORTB |= (1 << PIN_LED_BLUE); }
static inline void io_led_blue_off(void) { PORTB &= ~(1 << PIN_LED_BLUE); }

static inline void io_led_red(bool state) {
	if (state)
		io_led_red_on();
	else
		io_led_red_off();
}

static inline void io_led_green(bool state) {
	if (state)
		io_led_green_on();
	else
		io_led_green_off();
}

static inline void io_led_blue(bool state) {
	if (state)
		io_led_blue_on();
	else
		io_led_blue_off();
}

static inline bool io_led_red_state(void) { return (PORTD >> PIN_LED_RED) & 0x1; }
static inline bool io_led_green_state(void) { return (PORTB >> PIN_LED_GREEN) & 0x1; }
static inline bool io_led_blue_state(void) { return (PORTB >> PIN_LED_BLUE) & 0x1; }

static inline void io_led_red_toggle(void) { io_led_red(!io_led_red_state()); }
static inline void io_led_green_toggle(void) { io_led_green(!io_led_green_state()); }
static inline void io_led_blue_toggle(void) { io_led_blue(!io_led_blue_state()); }

static inline bool io_button(void) { return (PIND >> PIN_BUTTON) & 0x1; }

static inline void uart_out(void) { PORTD |= (1 << PIN_UART_DIR); }
static inline void uart_in(void) { PORTD &= ~(1 << PIN_UART_DIR); }

#endif
