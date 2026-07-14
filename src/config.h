#ifndef _CONFIG_H_
#define _CONFIG_H_

/* General configuration of MTB-LED module
 */

#include <stdint.h>
#include <stdbool.h>
#include "io.h"

///////////////////////////////////////////////////////////////////////////////

extern bool config_write; // request to write config to EEPROM

///////////////////////////////////////////////////////////////////////////////
// Configuration variables

typedef struct {
	uint32_t safe_state;
	uint8_t pwm[NO_OUTPUTS];
	uint8_t mtbbus_addr;
	uint8_t mtbbus_speed;
} config_t;

extern config_t config;

#define MTBBUS_CONFIG_SIZE (sizeof(config.safe_state)+sizeof(config.pwm))

///////////////////////////////////////////////////////////////////////////////

// Warning: these functions take long time to execute
void config_load(void);
bool config_save(void);

void config_boot_fwupgd(void);
void config_boot_normal(void);

void config_int_wdrf(bool value);
bool config_is_int_wdrf(void);

uint16_t config_bootloader_version(void);
uint8_t config_mcusr(void);

///////////////////////////////////////////////////////////////////////////////

#define CONFIG_MODULE_TYPE 0x40
#define CONFIG_FW_MAJOR 1
#define CONFIG_FW_MINOR 0
#define CONFIG_PROTO_MAJOR 4
#define CONFIG_PROTO_MINOR 1

#define CONFIG_BOOT_FWUPGD 0x01
#define CONFIG_BOOT_NORMAL 0x00

///////////////////////////////////////////////////////////////////////////////

#endif
