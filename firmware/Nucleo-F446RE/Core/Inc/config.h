
#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

#include "main.h"
#include <stdint.h>

/* AT24C256 I2C address with A2=A1=A0=GND (7-bit 0x50, shifted to 8-bit) */
#define CONFIG_I2C_ADDR  0xA0

#define CONFIG_MAGIC     0xC0FFEE01UL
#define CONFIG_VERSION   1

/* 64-byte config page stored at address 0x0000 in the AT24C256.
 * Do not access fields directly — use the getters and setters below. */
typedef struct {
	uint32_t magic;
	uint16_t version;
	uint16_t _reserved;
	float    hard_iron[3];       /* hard-iron offset vector (µT) */
	float    soft_iron[3][3];    /* soft-iron correction matrix; identity = uncorrected */
	uint8_t  _pad[4];
	uint16_t crc;                /* CRC-16/CCITT over bytes 0..offsetof(crc)-1 */
} config_data_t;

typedef struct {
	I2C_HandleTypeDef *port;
	uint16_t           i2c_addr;
	void             (*on_log)(const char *fmt, ...);
	config_data_t      data;
} config_t;

/**
 * config_set_log_callback
 * @param p        - Pointer to an initialised config_t struct
 * @param callback - Function to call for log output, or NULL to disable
 *
 * Registers a logging callback with the same signature as app_log().
 * Pass app_log directly.  Must be called before config_load() to capture
 * startup messages.
 */
void config_set_log_callback(config_t *p, void (*callback)(const char *fmt, ...));

/**
 * config_init
 * @param p        - Pointer to the config_t struct to populate
 * @param port     - I2C bus the AT24C256 is connected to
 * @param i2c_addr - 8-bit I2C address (CONFIG_I2C_ADDR)
 *
 * Stores handles and loads factory defaults into RAM.  No I2C traffic.
 * Call before config_load().
 */
void config_init(config_t *p, I2C_HandleTypeDef *port, uint16_t i2c_addr);

/**
 * config_load
 * @param p - Pointer to an initialised config_t struct
 * @return HAL_OK if the EEPROM page was read and passed magic + CRC checks.
 *         HAL_ERROR if the EEPROM was blank, corrupt, or unreadable — in
 *         which case factory defaults remain in RAM and the condition is
 *         logged via app_log().
 *
 * Reads the 64-byte config page from the AT24C256 and validates it.
 * Call once from app_init() after all MX_*_Init() calls.
 */
HAL_StatusTypeDef config_load(config_t *p);

/**
 * config_get_mag_hard_iron
 * @param p     - Pointer to an initialised config_t struct
 * @param out_x - Hard-iron X offset (µT)
 * @param out_y - Hard-iron Y offset (µT)
 * @param out_z - Hard-iron Z offset (µT)
 *
 * Returns the hard-iron offset vector from RAM.  No I2C traffic.
 */
void config_get_mag_hard_iron(const config_t *p, float *out_x, float *out_y, float *out_z);

/**
 * config_get_mag_soft_iron
 * @param p          - Pointer to an initialised config_t struct
 * @param out_matrix - Destination 3×3 matrix; receives a copy of the
 *                     soft-iron correction matrix from RAM
 *
 * No I2C traffic.
 */
void config_get_mag_soft_iron(const config_t *p, float out_matrix[3][3]);

/**
 * config_set_mag_hard_iron
 * @param p - Pointer to an initialised config_t struct
 * @param x - Hard-iron X offset (µT)
 * @param y - Hard-iron Y offset (µT)
 * @param z - Hard-iron Z offset (µT)
 * @return HAL_OK on a successful EEPROM write
 *
 * Updates the RAM copy and writes the full 64-byte config page to the
 * AT24C256, polling for write-cycle completion (max ~10 ms).
 */
HAL_StatusTypeDef config_set_mag_hard_iron(config_t *p, float x, float y, float z);

/**
 * config_set_mag_soft_iron
 * @param p      - Pointer to an initialised config_t struct
 * @param matrix - 3×3 soft-iron correction matrix to store
 * @return HAL_OK on a successful EEPROM write
 *
 * Updates the RAM copy and writes the full 64-byte config page to the
 * AT24C256, polling for write-cycle completion (max ~10 ms).
 */
HAL_StatusTypeDef config_set_mag_soft_iron(config_t *p, const float matrix[3][3]);

#endif /* INC_CONFIG_H_ */
