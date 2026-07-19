/* TLC5940 interface implementation.
 */

#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <stdbool.h>
#include <string.h>
#include "tlc5940.h"
#include "io.h"
#include "config.h"
#include "diag.h"

///////////////////////////////////////////////////////////////////////////////

#define TLC_OUT_BUF_SIZE 48 // NO_OUTPUTS * 1.5 (each output is 12 bits)

uint32_t tlc_outputs_want_state = 0;
uint32_t tlc_outputs_real_state = 0;
uint32_t tlc_outputs_connected = 0;
uint8_t _buf_out[TLC_OUT_BUF_SIZE];
volatile bool tlc_update_request;

const uint8_t _OUTPUT_MAP[NO_OUTPUTS] = {
	 7,  6,  5,  4,  3,  2,  1,  0,
	31, 30, 29, 28, 27, 26, 25, 24,
	23, 22, 21, 20, 19, 18, 17, 16,
	15, 14, 13, 12, 11, 10,  9,  8
};

volatile struct {
	bool sample_request;
	bool send_request;
} _flags;

///////////////////////////////////////////////////////////////////////////////

static void _out_spi_send(void);
static void _prepare_out_data(uint32_t state);
void _sample_status(void);

///////////////////////////////////////////////////////////////////////////////

void tlc_init(uint32_t out_state) {
	memset((void*)&_flags, 0, sizeof(_flags));

	// Setup pins
	DDRD |= (1 << PIN_GSCLK);
	PORTD |= (1 << PIN_GSCLK); // PORT must be active for CTC mode output, see datasheet p. 166
	PORTB |= (1 << PIN_BLANK); // start with BLANK high (external pullup on PCB)
	DDRB |= (1 << PIN_BLANK) | (1 << PIN_XLAT);
	DDRE |= (1 << PE3) | (1 << PE2); // MOSI1 & SS1 out
	DDRC |= (1 << PC1); // SCK1 out
	PORTC |= (1 << PC0); // pull-up on MISO just for sure

	// Setup timer 1 for BLANK (& XLAT in some situations - see below)
	TCCR1B = (1 << WGM13); // Phase/freq correct PWM, ICR1 top
	OCR1A = 1; // duty factor on XLAT
	OCR1B = 2; // duty factor on BLANK
	ICR1 = 4096;

	// Setup SPI1
	SPSR1 = (1 << SPI2X1);
	SPCR1 = (1 << SPE1) | (1 << MSTR1); // enable SPI, master mode, frequency=f_osc/2

	// Setup timer 4 @ ~1.054 MHz (GSCLK pin)
	TCCR4A = (1 << COM4B0); // OC4B toggles output pin PD2
	TCCR4B = (1 << WGM42); // CTC mode
	OCR4A = 0; // as-fast-as-possible
	TCCR4B |= (1 << CS40); // start timer, no prescaler

	// do not call tlc_out_set, call '_prepare_out_data' & '_out_spi_send' right now, do not wait for 'tlc_update'
	tlc_outputs_want_state = out_state;
	_prepare_out_data(out_state);
	_out_spi_send();

	io_xlat_on(); // trigger XLAT manually
	io_xlat_off();
	TCCR1A = (1 << COM1B1); // connect BLANK, non inverting, Clear OC1A/OC1B on Compare Match when up-counting. Set OC1A/OC1B on Compare Match when down-counting.
	TCCR1B |= (1 << CS10); // start timer
	_flags.sample_request = true;
}

void tlc_update(void) {
	if (_flags.send_request) {
		_flags.send_request = false;
		_out_spi_send();
	} else if (_flags.sample_request) {
		_flags.sample_request = false;
		_sample_status();
	} else if (tlc_update_request) {
		tlc_update_request = false;
		_out_spi_send(); // no need to _prepare_out_data
	}
}

/* Output setting has 3 stages:
 * 1) _prepare_out_data
 * 2) _out_spi_send
 * 3) _sample_status
 * Once a stage finishes, next stage in executed in next call of 'tlc_update'.
 * This is because each stage takes non-trivial time (see function's docstrings)
 * and we can't block MCU for long time, because it needs to handle MTBbus communication
 * continuously.
 */
void tlc_out_set(uint32_t state) {
	tlc_outputs_want_state = state;
	tlc_outputs_real_state = (error_flags.bits.mcutemp_critical) ? 0 : state;
	_prepare_out_data(tlc_outputs_real_state);
	_flags.send_request = true;
}

/* Prepare data for TLC5940 into '_buf_out' based on 'state' */
void _prepare_out_data(uint32_t state) {
	memset(_buf_out, 0, sizeof(_buf_out));

	// need to process 2 outputs in one iteration, because each output is 12 bits
	uint32_t _outputs = (state << 24) | (state >> 8);
	uint8_t bufi = 0;
	for (uint8_t i = 0; i < NO_OUTPUTS; i += 2) {
		if (_outputs&0x80000000) {
			_buf_out[bufi] = config.pwm[_OUTPUT_MAP[i]];
		}
		if (_outputs&0x40000000) {
			const uint8_t pwm = config.pwm[_OUTPUT_MAP[i+1]];
			_buf_out[bufi+1] = pwm >> 4;
			_buf_out[bufi+2] = pwm << 4;
		}
		_outputs <<= 2;
		bufi += 3;
	}
}

/* Physically send data to TLC5940 over SPI (blocking).
 * This function should be as-fast-as-possible because it sends data to all 32 outputs
 * in one blocking call.
 * Typical duration of this function: 100 us.
 */
void _out_spi_send(void) {
	SPDR1 = _buf_out[0];
	for (uint8_t i = 1; i < sizeof(_buf_out); i++) {
		while (!(SPSR1 & (1<<SPIF1)));
		SPDR1 = _buf_out[i];
	}
	while (!(SPSR1 & (1<<SPIF1)));

	// On next BLANK cycle, trigger also XLAT and TIMER1_OVF_vect interrupt, which triggers '_sample_status'
	if ((TCCR1B & 0x07) > 0) { // if timer is running (not in call from tlc_init)
		while (TCNT1 <= (OCR1B+100)); // wait for timer in state XLAT=LOW, BLANK=LOW (phase/freq correct PWM mode used - TCNT1 goes up and down)
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			TIFR1 |= (1 << TOV1); // clear interrupt flag
			TIMSK1 = (1 << TOIE1); // enable interrupt
			TCCR1A |= (1 << COM1A1); // enable XLAT signal
		}
	}
}

/* Receive TLC5940 status (e.g. Status Register) (blocking).
 * Typical duration of this function: 60 us.
 */
void _sample_status(void) {
	const uint8_t SECOND_TLC_I = TLC_OUT_BUF_SIZE/2;
	const uint8_t BUF_IN_SIZE = SECOND_TLC_I+3; // we don't need full data, read just part relevant for us

	// ----------- Perform SPI read -----------
	SPCR1 |= (1 << CPHA1);
	uint8_t buf_in[BUF_IN_SIZE];
	SPDR1 = 0;
	for (uint8_t i = 0; i < sizeof(buf_in); i++) {
		while (!(SPSR1 & (1<<SPIF1)));
		buf_in[i] = SPDR1;
		SPDR1 = 0;
	}
	while (!(SPSR1 & (1<<SPIF1)));
	SPCR1 &= ~(1 << CPHA1);

	// ----------- Process SPI in data -----------
	uint32_t outputs_lod = buf_in[0] | ((uint32_t)buf_in[1] << 24) | ((uint32_t)buf_in[SECOND_TLC_I] << 16) | ((uint32_t)buf_in[SECOND_TLC_I+1] << 8);
	tlc_outputs_connected = (~outputs_lod) & tlc_outputs_real_state;
	mtbbus_warn_flags.bits.tlc_tef = (buf_in[2] != 0) || (buf_in[SECOND_TLC_I+2] != 0);
	mtbbus_warn_flags.bits.tlc_lod = (tlc_outputs_connected != tlc_outputs_real_state);
}

ISR(TIMER1_OVF_vect) {
	TIMSK1 &= ~(1 << TOIE1); // disable XLAT signal
	TCCR1A &= ~(1 << COM1A1); // disable interrupt
	_flags.sample_request = true;
}

