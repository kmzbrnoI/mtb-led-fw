#include <avr/io.h>
#include <util/delay.h>

#include "io.h"
#include "common.h"

///////////////////////////////////////////////////////////////////////////////

static inline void io_reset(void) {
	SPCR0 = 0;
	DDRB = 0;
	DDRC = 0;
	DDRD = 0;
	DDRE = 0;
	PORTB = 0;
	PORTC = 0;
	PORTD = 0;
	PORTE = 0;
}

void io_init() {
	// Reset (could load from bootloader)
	io_reset();

	DDRB |= (1 << PIN_LED_GREEN) | (1 << PIN_LED_BLUE);
	DDRD |= (1 << PIN_LED_RED);
	DDRE |= (1 << PIN_DEBUG1) | (1 << PIN_DEBUG2);
	PORTD |= (1 << PIN_BUTTON) | (1 << PIN_UART_RX); // pull-ups

	DDRB |= (1 << PB3) | (1 << PB5) | (1 << PB2); // MOSI & SCK & SS out
	PORTB |= (1 << PB4); // pull-up on MISO just for sure
}

