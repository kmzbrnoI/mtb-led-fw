#include <stdbool.h>
#include <string.h>
#include "outputs.h"
#include "io.h"

void out_init(void) {
	DDRD |= (1 << PIN_GSCLK);
	PORTD |= (1 << PIN_GSCLK); // PORT must be active for CTC mode output, see datasheet p. 166

	// Setup timer 4 @ ~1.054 MHz (GSCLK pin)
	TCCR4A = (1 << COM4B0); // OC4B toggles output pin PD2
	TCCR4B = (1 << CS40) | (1 << WGM42); // CTC mode, no prescaler
	OCR4A = 6;

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


void io_set_output_raw(uint8_t onum, bool state);
void io_set_outputs_raw(uint16_t state);
void io_set_outputs_raw_mask(uint16_t state, uint16_t mask);
uint16_t io_get_outputs_raw(void);

bool io_get_output_raw(uint8_t onum);
