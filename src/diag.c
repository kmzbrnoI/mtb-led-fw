#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include "diag.h"

/* Diagnostic
 * Measurement of MCU voltage & temperature is highly inaccurate. V_BG should be
 * 1.1 V (1.0-1.2 V according to datasheet), however 0.950 V was measured in
 * some casses in practise. This is unacceptable error -> voltage and temperature
 * measurement was discarded.
 * Temperature sensor has additional error, it should be calibrated etc.
 */

///////////////////////////////////////////////////////////////////////////////
// Global variables

mcusr_t mcusr;
error_flags_t error_flags = {0};
mtbbus_warn_flags_t mtbbus_warn_flags = {0};
mtbbus_warn_flags_t mtbbus_warn_flags_old = {0};
volatile uint16_t init_vcc = 0xFFFF;
volatile uint32_t uptime_seconds = 0;

///////////////////////////////////////////////////////////////////////////////
// Function prototypes

static inline void adc_start(void);

///////////////////////////////////////////////////////////////////////////////

void diag_init(void) {
	ADCSRA = (1 << ADIE) | (1 << ADEN); // enable ADC interrupt, enable ADC
	ADCSRA |= 0x5; // prescaler 32×
	ADMUX = (1 << REFS0); // AVCC with external capacitor at AREF pin
	ADMUX |= 0x0E; // measure internal 1V1 band-gap reference
}

void diag_update(void) {
	// called each 100 ms
	adc_start();

	{
		static uint8_t uptime_counter = 0;
		uptime_counter++;
		if (uptime_counter >= 10) {
			uptime_seconds++;
			uptime_counter = 0;
		}
	}
}

void adc_start(void) {
	ADCSRA |= (1 << ADSC); // start conversion
}

ISR(ADC_vect) {
	uint16_t value = ADCL;
	value |= (ADCH << 8);

	if (init_vcc == 0xFFFF)
		init_vcc = value;

	uint16_t diff = value > init_vcc ? value-init_vcc : init_vcc-value;
	if (diff > VCC_MAX_DIFF)
		mtbbus_warn_flags.bits.vcc_oscilating = true;
}
