#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include <string.h>
#include "diag.h"
#include "config.h"
#include "tlc5940.h"

/* Diagnostic
 * Measurement of MCU voltage & temperature is highly inaccurate. V_BG should be
 * 1.1 V (1.0-1.2 V according to datasheet), however 0.950 V was measured in
 * some casses in practise. Thus, temperature single-point calibration is performed
 * as part of the MTB-LED manufacturing process. TS_OFFSET in stored in EEPROM and programmed
 * via module-specific command 0x01 over MTBbus (see main.c).
 * We need to precisely measure the temperature, because TLC5940s could get quite hot
 * (100 °C measured in edge case in practise), so increased MCU temperature indicates
 * possible problem with TLC5940 overheating. TLC's internal TOF threshold is set to ~150 °C,
 * which is quite high temperature, we want to indicate possible problem sooner.
 *
 * Voltage measurement is dropper for now.
 */

///////////////////////////////////////////////////////////////////////////////
// Global variables

mcusr_t mcusr;
error_flags_t error_flags = {0};
error_flags_t error_flags_old = {0};
mtbbus_warn_flags_t mtbbus_warn_flags = {0};
mtbbus_warn_flags_t mtbbus_warn_flags_old = {0};
volatile uint32_t uptime_seconds = 0;
tempmeas_t mcutemp;

///////////////////////////////////////////////////////////////////////////////
// Function prototypes

static inline void adc_start(void);
static void mcutemp_limits_check(void);

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

	mcutemp_limits_check(); // based on data from last ADC measurement
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
	mcutemp.degc = (int16_t)value + config.ts_offset;
}

void mcutemp_limits_check(void) {
	if (mtbbus_warn_flags.bits.ts_offset_uncalibrated)
		return;

	if (mtbbus_warn_flags.bits.mcutemp_high) {
		if (mcutemp.degc <= MCUTEMP_HIGH_WARNING_OFF_THRESHOLD)
			mtbbus_warn_flags.bits.mcutemp_high = false;
	} else {
		if (mcutemp.degc >= MCUTEMP_HIGH_WARNING_ON_THRESHOLD)
			mtbbus_warn_flags.bits.mcutemp_high = true;
	}

	if (error_flags.bits.mcutemp_critical) {
		if (mcutemp.degc <= MCUTEMP_HIGH_ERROR_OFF_THRESHOLD) {
			error_flags.bits.mcutemp_critical = false;
			tlc_out_set(tlc_outputs_want_state);
		}
	} else {
		if (mcutemp.degc >= MCUTEMP_HIGH_ERROR_ON_THRESHOLD) {
			error_flags.bits.mcutemp_critical = true;
			tlc_out_set(tlc_outputs_want_state);
		}
	}
}
