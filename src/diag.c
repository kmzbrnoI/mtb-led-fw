#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include <string.h>
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
volatile uint32_t uptime_seconds = 0;
tempmeas_t mcutemp;

///////////////////////////////////////////////////////////////////////////////
// Function prototypes

static inline void adc_start(void);

///////////////////////////////////////////////////////////////////////////////

void diag_init(void) {
	memset(&mcutemp, 0, sizeof(mcutemp));

	ADCSRA = (1 << ADIE) | (1 << ADEN); // enable ADC interrupt, enable ADC
	ADCSRA |= 0x5; // prescaler 32×
	ADMUX = (1 << REFS0) | (1 << REFS1); // 1V1 internal reference with ext. capacitor at AREF pin
	ADMUX |= 0x08; // measure ADC8 = temperature sensor
}

void diag_update(void) {
	// called each 100 ms

	adc_start(); // measure temperature

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

	mcutemp.raw = value;
	mcutemp.degc = (int16_t)value-290;
}
