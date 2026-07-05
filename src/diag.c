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
volatile uint32_t uptime_seconds = 0;

///////////////////////////////////////////////////////////////////////////////
// Function prototypes

///////////////////////////////////////////////////////////////////////////////

void diag_init(void) {
}

void diag_update(void) {
	// called each 100 ms

	{
		static uint8_t uptime_counter = 0;
		uptime_counter++;
		if (uptime_counter >= 10) {
			uptime_seconds++;
			uptime_counter = 0;
		}
	}
}
