#include <util/delay.h>
#include <stdbool.h>
#include <string.h>
#include "tlc5940.h"
#include "io.h"
#include "config.h"

///////////////////////////////////////////////////////////////////////////////

uint32_t tlc_outputs_state = 0;
const uint8_t OUTPUT_MAP[NO_OUTPUTS] = {
	 7,  6,  5,  4,  3,  2,  1,  0,
	31, 30, 29, 28, 27, 26, 25, 24,
	23, 22, 21, 20, 19, 18, 17, 16,
	15, 14, 13, 12, 11, 10,  9,  8
};

///////////////////////////////////////////////////////////////////////////////

static void _out_spi_send(void);
static inline void _xlat_enable(void);
static inline void _xlat_disable(void);

///////////////////////////////////////////////////////////////////////////////

void tlc_init(uint32_t out_state) {
	// Setup pins
	DDRD |= (1 << PIN_GSCLK);
	PORTD |= (1 << PIN_GSCLK); // PORT must be active for CTC mode output, see datasheet p. 166
	PORTB |= (1 << PIN_BLANK) | (1 << PIN_XLAT); // start with BLANK&XLAT high
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

	tlc_out_set(out_state);

	TCCR1B |= (1 << CS10); // start timer, no prescaler
	TCCR4B |= (1 << CS40); // start timer, no prescaler
	PORTB &= ~(1 << PIN_XLAT); // when _xlat_disable caller, signal must go to LOW
}

void tlc_out_set(uint32_t state) {
	tlc_outputs_state = state;
	_xlat_disable(); // do not propagate to GS register during SPI shift
	_out_spi_send();
	_xlat_enable(); // propagate to GS register on next proper cycle
}

void _out_spi_send(void) {
	// Typical time of this function: 150 us
	SPDR0 = 0; // so first while does not loop

	uint8_t buf[48]; // NO_OUTPUTS * 1.5 (each output is 12 bits)
	memset(buf, 0, sizeof(buf));

	// need to process 2 outputs in one iteration, because each output is 12 bits
	uint32_t _outputs = (tlc_outputs_state << 24) | (tlc_outputs_state >> 8);
	uint8_t bufi = 0;
	for (uint8_t i = 0; i < NO_OUTPUTS; i += 2) {
		if (_outputs&0x80000000) {
			buf[bufi] = config_pwm[OUTPUT_MAP[i]];
		}
		if (_outputs&0x40000000) {
			const uint8_t pwm = config_pwm[OUTPUT_MAP[i+1]];
			buf[bufi+1] = pwm >> 4;
			buf[bufi+2] = pwm << 4;
		}
		_outputs <<= 2;
		bufi += 3;
	}

	for (uint8_t i = 0; i < sizeof(buf); i++) {
		while (!(SPSR0 & (1<<SPIF)));
		SPDR0 = buf[i];
	}

	while (!(SPSR0 & (1<<SPIF)));
}

void _xlat_enable(void) {
	TCCR1A |= (1 << COM1A1);
}

void _xlat_disable(void) {
	TCCR1A &= ~(1 << COM1A1);
}
