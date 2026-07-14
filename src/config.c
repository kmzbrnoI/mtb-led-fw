#include <avr/eeprom.h>
#include <string.h>
#include "config.h"
#include "mtbbus.h"

config_t config;
bool config_write = false;

#define EEPROM_ADDR_VERSION                ((uint8_t*)0x00)
#define EEPROM_ADDR_MTBBUS_SPEED           ((uint8_t*)0x01)
#define EEPROM_ADDR_INT_WDRF               ((uint8_t*)0x02)
#define EEPROM_ADDR_BOOT                   ((uint8_t*)0x03)
#define EEPROM_ADDR_MTBBUS_ADDR            ((uint8_t*)0x04)
#define EEPROM_ADDR_BOOTLOADER_VER_MAJOR   ((uint8_t*)0x08)
#define EEPROM_ADDR_BOOTLOADER_VER_MINOR   ((uint8_t*)0x09)
#define EEPROM_ADDR_BOOTLOADER_MCUSR       ((uint8_t*)0x0A)
#define EEPROM_ADDR_SAFE_STATE             ((uint32_t*)0x10)
#define EEPROM_ADDR_PWM                    ((uint8_t*)0x20)


void config_load(void) {
	uint8_t version = eeprom_read_byte(EEPROM_ADDR_VERSION);
	if (version == 0xFF) {
		// default EEPROM content → reset config
		config.mtbbus_speed = MTBBUS_SPEED_38400;
		config.mtbbus_addr = 1;
		config.safe_state = 0;
		memset(config.pwm, 100, sizeof(config.pwm));
		while (!config_save()); // loop until everything saved
		return;
	}

	config.mtbbus_speed = eeprom_read_byte(EEPROM_ADDR_MTBBUS_SPEED);
	if (config.mtbbus_speed > MTBBUS_SPEED_MAX)
		config.mtbbus_speed = MTBBUS_SPEED_38400;

	config.mtbbus_addr = eeprom_read_byte(EEPROM_ADDR_MTBBUS_ADDR);
	if (config.mtbbus_addr == 0) // address 0 is invalid (it is broadcast)
		config.mtbbus_addr = 1;

	uint8_t boot = eeprom_read_byte(EEPROM_ADDR_BOOT);
	if (boot != CONFIG_BOOT_NORMAL)
		eeprom_write_byte(EEPROM_ADDR_BOOT, CONFIG_BOOT_NORMAL);

	config.safe_state = eeprom_read_dword(EEPROM_ADDR_SAFE_STATE);
	eeprom_read_block(config.pwm, EEPROM_ADDR_PWM, sizeof(config.pwm));
}

bool config_save(void) {
	// Saves only 1 byte at a time, so this function never blocks for a long time (EEPROM busy wait).
	// Returns true iff all data have been succesfully saved.
	// If false is returned, call 'config_save' again to really save all data.

	if (!eeprom_is_ready()) return false;
	eeprom_update_byte(EEPROM_ADDR_VERSION, 1);
	if (!eeprom_is_ready()) return false;
	eeprom_update_byte(EEPROM_ADDR_MTBBUS_SPEED, config.mtbbus_speed);
	if (!eeprom_is_ready()) return false;
	eeprom_update_byte(EEPROM_ADDR_MTBBUS_ADDR, config.mtbbus_addr);

	for (uint8_t i = 0; i < sizeof(config.safe_state); i++) {
		if (!eeprom_is_ready()) return false;
		eeprom_update_byte((uint8_t*)EEPROM_ADDR_SAFE_STATE+i, *((uint8_t*)&config.safe_state+i));
	}

	for (uint8_t i = 0; i < sizeof(config.pwm); i++) {
		if (!eeprom_is_ready()) return false;
		eeprom_update_byte(EEPROM_ADDR_PWM+i, config.pwm[i]);
	}

	return eeprom_is_ready(); // true iff no write done
}

void config_boot_fwupgd(void) {
	eeprom_update_byte(EEPROM_ADDR_BOOT, CONFIG_BOOT_FWUPGD);
}

void config_boot_normal(void) {
	eeprom_update_byte(EEPROM_ADDR_BOOT, CONFIG_BOOT_NORMAL);
}

void config_int_wdrf(bool value) {
	eeprom_update_byte(EEPROM_ADDR_INT_WDRF, value);
}

bool config_is_int_wdrf(void) {
	return eeprom_read_byte(EEPROM_ADDR_INT_WDRF) & 1;
}

uint16_t config_bootloader_version(void) {
	uint16_t version = (eeprom_read_byte(EEPROM_ADDR_BOOTLOADER_VER_MAJOR) << 8) |
	                   (eeprom_read_byte(EEPROM_ADDR_BOOTLOADER_VER_MINOR));
	return version == 0xFFFF ? 0x0101 : version;
}

uint8_t config_mcusr(void) {
	return eeprom_read_byte(EEPROM_ADDR_BOOTLOADER_MCUSR);
}
