#include <util/delay.h>
#include <stdbool.h>
#include <string.h>
#include "outputs.h"
#include "io.h"
#include "config.h"

///////////////////////////////////////////////////////////////////////////////

uint32_t outputs_state = 0;

///////////////////////////////////////////////////////////////////////////////

static void _out_spi_send(void);
static inline void _spi_send_byte(uint8_t byte);
static inline void _xlat_enable(void);
static inline void _xlat_disable(void);

///////////////////////////////////////////////////////////////////////////////

void out_init(uint32_t state) {
	// Setup pins
	DDRD |= (1 << PIN_GSCLK);
	PORTD |= (1 << PIN_GSCLK); // PORT must be active for CTC mode output, see datasheet p. 166
	DDRB |= (1 << PIN_BLANK) | (1 << PIN_XLAT);
	DDRC |= (1 << PIN_VPRG) | (1 << PIN_DCPRG);
	io_vprg_off();
	io_dcprg_off();

	// Setup timer 1 for XLAT & BLANK
	TCCR1A = (1 << COM1A1) | (1 << COM1B1); // non inverting, Clear OC1A/OC1B on Compare Match when up-counting. Set OC1A/OC1B on Compare Match when down-counting.
	TCCR1B = (1 << WGM13); // Phase/freq correct PWM, ICR1 top
	OCR1A = 1; // duty factor on OC1A, XLAT is inside BLANK
	OCR1B = 2; // duty factor on BLANK (larger than OCR1A (XLAT))
	ICR1 = 8192;

	// Setup timer 4 @ ~1.054 MHz (GSCLK pin)
	TCCR4A = (1 << COM4B0); // OC4B toggles output pin PD2
	TCCR4B = (1 << WGM42); // CTC mode
	OCR4A = 0; // as-fast-as-possible

	// Setup SPI
	SPSR0 = (1 << SPI2X);
	SPCR0 = (1 << SPE) | (1 << MSTR); // enable SPI, master mode, frequency=f_osc/2

	out_set(state);

	PORTB |= (1 << PIN_BLANK) | (1 << PIN_XLAT); // start with BLANK&XLAT high
	TCCR1B |= (1 << CS10); // start timer, no prescaler
	TCCR4B |= (1 << CS40); // start timer, no prescaler
	PORTB &= ~(1 << PIN_XLAT); // when _xlat_disable caller, signal must go to LOW
}

void out_set(uint32_t state) {
	outputs_state = state;
	_xlat_disable(); // do not propagate to GS register during SPI shift
	_out_spi_send();
	_xlat_enable(); // propagate to GS register on next proper cycle
}

void _spi_send_byte(uint8_t byte) {
	SPDR0 = byte;
	while (!(SPSR0 & (1<<SPIF)));
}

void _out_spi_send(void) {
	// Typical time of this function: ?? us
	uint32_t state = outputs_state;

	// need to send 2 outputs in one iteration, because each output is 12 bits
	for (size_t outi = 0; outi < NO_OUTPUTS; outi += 2) {
		_spi_send_byte((state&1) ? config_pwm[outi] : 0);
		_spi_send_byte(0); // TODO better
		_spi_send_byte((state&2) ? config_pwm[outi+1] : 0);
		state >>= 2;
	}
}

void _xlat_enable(void) {
	TCCR1A |= (1 << COM1A1);
}

void _xlat_disable(void) {
	TCCR1A &= ~(1 << COM1A1);
}
