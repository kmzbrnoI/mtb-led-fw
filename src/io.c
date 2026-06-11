#include <avr/io.h>
#include <util/delay.h>

#include "io.h"
#include "common.h"

///////////////////////////////////////////////////////////////////////////////

volatile uint16_t _outputs = 0;

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

	//SPCR = (1 << SPE) | (1 << MSTR) | (1 << CPOL); // enable SPI, SPI master, frequency=f_osc/4
}

/*void io_shift_update() {
	// Typical time of this function: 10 us

	uint16_t buf_inputs = 0;
	uint8_t read;

	// Input load
	PORTD |= (1 << PIN_INPUT_SHIFT);
	__asm__("nop");
	__asm__("nop");

	SPDR = 0;
	while (!(SPSR & (1<<SPIF)));
	read = SPDR;
	buf_inputs = switch_bits_03(read);

	SPDR = _outputs & 0xFF;
	while (!(SPSR & (1<<SPIF)));
	read = SPDR;
	buf_inputs |= switch_bits_03(read) << 8;

	SPDR = _outputs >> 8;
	while (!(SPSR & (1<<SPIF)));
	read = ~SPDR;

	// Address is connected to shift's input almost randomly... :(
	_address = ((read & 0xF0) >> 2) | ((read & 0x8) >> 3) | ((read & 0x4) >> 1)
		| ((read & 0x2) << 5) | ((read & 0x1) << 7);

	// Output set
	PORTD |= (1 << PIN_OUTPUT_SET);
	__asm__("nop");
	__asm__("nop");
	PORTD &= ~(1 << PIN_OUTPUT_SET);

	PORTD &= ~(1 << PIN_INPUT_SHIFT);

	_inputs = buf_inputs;
}

void io_shift_load() {
	PORTD &= ~(1 << PIN_INPUT_SHIFT);
	__asm__("nop");
	__asm__("nop");
	PORTD |= (1 << PIN_INPUT_SHIFT); // disable loading now, data stays loaded in shifts
}*/

