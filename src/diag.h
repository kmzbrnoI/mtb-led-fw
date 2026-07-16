#ifndef _DIAG_H_
#define _DIAG_H_

/* Diagnostics */

#include <stdbool.h>
#include <stdint.h>

#define DIAG_UPDATE_PERIOD 10 // 100 ms
#define MCUTEMP_HIGH_WARNING_ON_THRESHOLD 50 // [°C] quite low number, but this indicates e.g. 100 °C of TLC5940
#define MCUTEMP_HIGH_WARNING_OFF_THRESHOLD 45 // [°C]
#define MCUTEMP_HIGH_ERROR_ON_THRESHOLD 60 // [°C]
#define MCUTEMP_HIGH_ERROR_OFF_THRESHOLD 55 // [°C]

typedef union {
	struct {
		bool porf : 1;
		bool extrf : 1;
		bool borf : 1;
		bool wdrf : 1;
	} bits;
	uint8_t all;
} mcusr_t;

extern mcusr_t mcusr;


typedef union {
	struct {
		bool _UNUSED : 1; // was 'addr_zero'
		bool bad_mtbbus_polarity : 1;
		bool mcutemp_critical : 1;
	} bits;
	uint8_t all;
} error_flags_t;

extern error_flags_t error_flags;
extern error_flags_t error_flags_old;


typedef union {
	struct {
		bool extrf : 1;
		bool borf : 1;
		bool wdrf : 1;
		bool _ : 1;
		bool missed_timer : 1;
		uint8_t __ : 3; // padding
		bool tlc_tef : 1; // TLC5940 thermal error flag
		bool tlc_lod : 1; // TLC5940 LED open flag
		bool ts_offset_uncalibrated : 1; // temperature sensor offset uncalibrated (need calubration after manufacturing)
		bool mcutemp_high : 1;
	} bits;
	uint16_t all;
} mtbbus_warn_flags_t;

extern mtbbus_warn_flags_t mtbbus_warn_flags;
extern mtbbus_warn_flags_t mtbbus_warn_flags_old;

typedef struct {
	volatile uint16_t raw;
	volatile int16_t degc; // [degrees celsius]
} tempmeas_t;

extern tempmeas_t mcutemp;

extern volatile uint32_t uptime_seconds;

///////////////////////////////////////////////////////////////////////////////

void diag_init(void);
void diag_update(void); // called each 100 ms
void vcc_start_measure(void);


#endif
