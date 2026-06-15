
#ifndef INC_LSM6DSRX_H_
#define INC_LSM6DSRX_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* Register map (LSM6DSRX datasheet section 8/9) */
#define LSM6DSRX_REG_WHO_AM_I    0x0F
#define LSM6DSRX_REG_CTRL1_XL    0x10
#define LSM6DSRX_REG_CTRL2_G     0x11
#define LSM6DSRX_REG_CTRL3_C     0x12
#define LSM6DSRX_REG_STATUS_REG  0x1E
#define LSM6DSRX_REG_OUTX_L_G    0x22
#define LSM6DSRX_REG_OUTX_L_A    0x28

/* WHO_AM_I is fixed at 0x6B on the LSM6DSRX, 0x6C on the pin/register
 * compatible LSM6DSOX - bringup accepts either. */
#define LSM6DSRX_WHOAMI_LSM6DSRX 0x6B
#define LSM6DSRX_WHOAMI_LSM6DSOX 0x6C

/* I2C addresses selectable via the SDO/SA0 pin */
#define LSM6DSRX_I2C_ADDR_SA0_LOW  0x6A
#define LSM6DSRX_I2C_ADDR_SA0_HIGH 0x6B

typedef struct lsm6dsrx_s {
	I2C_HandleTypeDef *port;
	uint16_t address;
	void (*accel_ready)(struct lsm6dsrx_s *p, int16_t x, int16_t y, int16_t z);
	void (*gyro_ready)(struct lsm6dsrx_s *p, int16_t x, int16_t y, int16_t z);
} lsm6dsrx_t;

/**
 * lsm6dsrx_init
 * @param p - Pointer to the lsm6dsrx_t struct to populate
 * @param in_port - I2C bus the device is connected to
 * @param in_address - 7-bit I2C address to probe first in
 *                      lsm6dsrx_bringup() (e.g. LSM6DSRX_I2C_ADDR_SA0_LOW)
 *
 * Populates the struct only - no I2C traffic is generated.
 */
void lsm6dsrx_init(lsm6dsrx_t *p, I2C_HandleTypeDef *in_port, uint16_t in_address);

/**
 * lsm6dsrx_bringup
 * @param p - Pointer to an initialized lsm6dsrx_t struct
 * @return HAL_OK if WHO_AM_I matched LSM6DSRX or LSM6DSOX and the
 *         accelerometer/gyroscope were configured successfully
 *
 * Reads WHO_AM_I at p->address; if it doesn't match either expected
 * value, retries at LSM6DSRX_I2C_ADDR_SA0_HIGH and updates p->address on
 * success. Logs the device/address found via app_log(). Configures
 * CTRL3_C (BDU + register auto-increment), CTRL1_XL and CTRL2_G
 * (104Hz ODR, accelerometer +/-2g, gyroscope +/-250dps).
 */
HAL_StatusTypeDef lsm6dsrx_bringup(lsm6dsrx_t *p);

/**
 * lsm6dsrx_data_ready
 * @param p - Pointer to an initialized lsm6dsrx_t struct
 * @param out_xl_ready - Set to true if new accelerometer data is available
 * @param out_g_ready - Set to true if new gyroscope data is available
 * @return HAL_OK on successful I2C read of STATUS_REG
 */
HAL_StatusTypeDef lsm6dsrx_data_ready(lsm6dsrx_t *p, bool *out_xl_ready, bool *out_g_ready);

/**
 * lsm6dsrx_read_accel
 * @param p - Pointer to an initialized lsm6dsrx_t struct
 * @param out_x - Raw accelerometer X output (OUTX_L_A/OUTX_H_A)
 * @param out_y - Raw accelerometer Y output (OUTY_L_A/OUTY_H_A)
 * @param out_z - Raw accelerometer Z output (OUTZ_L_A/OUTZ_H_A)
 * @return HAL_OK on successful I2C read
 */
HAL_StatusTypeDef lsm6dsrx_read_accel(lsm6dsrx_t *p, int16_t *out_x, int16_t *out_y, int16_t *out_z);

/**
 * lsm6dsrx_read_gyro
 * @param p - Pointer to an initialized lsm6dsrx_t struct
 * @param out_x - Raw gyroscope X output (OUTX_L_G/OUTX_H_G)
 * @param out_y - Raw gyroscope Y output (OUTY_L_G/OUTY_H_G)
 * @param out_z - Raw gyroscope Z output (OUTZ_L_G/OUTZ_H_G)
 * @return HAL_OK on successful I2C read
 */
HAL_StatusTypeDef lsm6dsrx_read_gyro(lsm6dsrx_t *p, int16_t *out_x, int16_t *out_y, int16_t *out_z);

/**
 * lsm6dsrx_set_accel_ready_callback
 * @param p - Pointer to an initialized lsm6dsrx_t struct
 * @param callback - Function to call when new accelerometer data is
 *                    available, or NULL to disable
 *
 * Registers a callback invoked by lsm6dsrx_loop() with the raw
 * accelerometer reading whenever STATUS_REG.XLDA is set. Stored in
 * p->accel_ready; NULL by default.
 */
void lsm6dsrx_set_accel_ready_callback(lsm6dsrx_t *p, void (*callback)(lsm6dsrx_t *p, int16_t x, int16_t y, int16_t z));

/**
 * lsm6dsrx_set_gyro_ready_callback
 * @param p - Pointer to an initialized lsm6dsrx_t struct
 * @param callback - Function to call when new gyroscope data is
 *                    available, or NULL to disable
 *
 * Registers a callback invoked by lsm6dsrx_loop() with the raw
 * gyroscope reading whenever STATUS_REG.GDA is set. Stored in
 * p->gyro_ready; NULL by default.
 */
void lsm6dsrx_set_gyro_ready_callback(lsm6dsrx_t *p, void (*callback)(lsm6dsrx_t *p, int16_t x, int16_t y, int16_t z));

/**
 * lsm6dsrx_loop
 * @param p - Pointer to an initialized lsm6dsrx_t struct
 * @return HAL_OK on successful I2C access
 *
 * Service handler - reads STATUS_REG and, for each of XLDA/GDA that is
 * set, reads the corresponding axes and invokes the registered
 * accel_ready/gyro_ready callback (if any). Intended to be called from
 * the idle loop, gated by a flags.h flag matching the configured ODR.
 */
HAL_StatusTypeDef lsm6dsrx_loop(lsm6dsrx_t *p);

#endif /* INC_LSM6DSRX_H_ */
