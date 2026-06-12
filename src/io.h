#ifndef _IO_H_
#define _IO_H_

/* Raw input-output operation of MTB-LED module.
 * This unit just abstracts IO with nice & fast functions.
 */

#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>

#define NO_OUTPUTS 32

#define PIN_LED_RED PD7
#define PIN_LED_GREEN PB0
#define PIN_LED_BLUE PB1

#define PIN_BUTTON PD5
#define PIN_DEBUG1 PE0
#define PIN_DEBUG2 PE1

#define PIN_UART_RX PD0
#define PIN_UART_TX PD1
#define PIN_UART_DIR PD3

#define PIN_VPRG PC1
#define PIN_DCPRG PD2
#define PIN_XLAT PC3
#define PIN_BLANK PC4
#define PIN_XERR PC5
#define PIN_GSCLK PD2

void io_init(void);

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

static inline void io_debug1_set(bool state) {
	if (state)
		PORTE |= (1 << PIN_DEBUG1);
	else
		PORTE &= ~(1 << PIN_DEBUG1);
}

static inline void io_debug2_set(bool state) {
	if (state)
		PORTE |= (1 << PIN_DEBUG2);
	else
		PORTE &= ~(1 << PIN_DEBUG2);
}

static inline bool io_debug1_get(void) { return ((PORTE >> PIN_DEBUG1) & 0x1); }
static inline void io_debug1_toggle(void) { io_debug1_set(!io_debug1_get()); }

static inline bool io_debug2_get(void) { return ((PORTE >> PIN_DEBUG2) & 0x1); }
static inline void io_debug2_toggle(void) { io_debug2_set(!io_debug2_get()); }

static inline void uart_out(void) { PORTD |= (1 << PIN_UART_DIR); }
static inline void uart_in(void) { PORTD &= ~(1 << PIN_UART_DIR); }

#endif
