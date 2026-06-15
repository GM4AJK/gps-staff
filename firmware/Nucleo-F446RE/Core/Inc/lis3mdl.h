
#ifndef INC_LIS3MDL_H_
#define INC_LIS3MDL_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* Register map (LIS3MDL datasheet section 6/7) */
#define LIS3MDL_REG_WHO_AM_I   0x0F
#define LIS3MDL_REG_CTRL_REG1  0x20
#define LIS3MDL_REG_CTRL_REG2  0x21
#define LIS3MDL_REG_CTRL_REG3  0x22
#define LIS3MDL_REG_CTRL_REG4  0x23
#define LIS3MDL_REG_STATUS_REG 0x27
#define LIS3MDL_REG_OUT_X_L    0x28

/* Set the SUB-address MSB to enable register auto-increment on
 * multi-byte I2C reads (datasheet section 5.1.1) */
#define LIS3MDL_AUTO_INCREMENT 0x80

/* WHO_AM_I is fixed at 0x3D */
#define LIS3MDL_WHOAMI 0x3D

/* I2C addresses selectable via the SDO/SA1 pin */
#define LIS3MDL_I2C_ADDR_SA1_LOW  0x1C
#define LIS3MDL_I2C_ADDR_SA1_HIGH 0x1E

typedef struct lis3mdl_s {
	I2C_HandleTypeDef *port;
	uint16_t address;
	void (*mag_ready)(struct lis3mdl_s *p, int16_t x, int16_t y, int16_t z);
} lis3mdl_t;

/* Running per-axis min/max of raw magnetometer readings, used by
 * lis3mdl_apply_calibration() for a min/max hard/soft-iron correction. */
typedef struct {
	int16_t min_x, max_x;
	int16_t min_y, max_y;
	int16_t min_z, max_z;
} lis3mdl_calibration_t;

/**
 * lis3mdl_init
 * @param p - Pointer to the lis3mdl_t struct to populate
 * @param in_port - I2C bus the device is connected to
 * @param in_address - 7-bit I2C address to probe first in
 *                      lis3mdl_bringup() (e.g. LIS3MDL_I2C_ADDR_SA1_LOW)
 *
 * Populates the struct only - no I2C traffic is generated.
 */
void lis3mdl_init(lis3mdl_t *p, I2C_HandleTypeDef *in_port, uint16_t in_address);

/**
 * lis3mdl_bringup
 * @param p - Pointer to an initialized lis3mdl_t struct
 * @return HAL_OK if WHO_AM_I matched and the magnetometer was
 *         configured successfully
 *
 * Reads WHO_AM_I at p->address; if it doesn't match LIS3MDL_WHOAMI,
 * retries at LIS3MDL_I2C_ADDR_SA1_HIGH and updates p->address on
 * success. Logs the address found via app_log(). Configures CTRL_REG1
 * (X/Y ultrahigh-performance, 10Hz ODR), CTRL_REG2 (+/-4 gauss),
 * CTRL_REG3 (continuous-conversion mode) and CTRL_REG4 (Z
 * ultrahigh-performance).
 */
HAL_StatusTypeDef lis3mdl_bringup(lis3mdl_t *p);

/**
 * lis3mdl_data_ready
 * @param p - Pointer to an initialized lis3mdl_t struct
 * @param out_ready - Set to true if a new X/Y/Z data set is available
 * @return HAL_OK on successful I2C read of STATUS_REG
 */
HAL_StatusTypeDef lis3mdl_data_ready(lis3mdl_t *p, bool *out_ready);

/**
 * lis3mdl_read_mag
 * @param p - Pointer to an initialized lis3mdl_t struct
 * @param out_x - Raw magnetometer X output (OUT_X_L/OUT_X_H)
 * @param out_y - Raw magnetometer Y output (OUT_Y_L/OUT_Y_H)
 * @param out_z - Raw magnetometer Z output (OUT_Z_L/OUT_Z_H)
 * @return HAL_OK on successful I2C read
 */
HAL_StatusTypeDef lis3mdl_read_mag(lis3mdl_t *p, int16_t *out_x, int16_t *out_y, int16_t *out_z);

/**
 * lis3mdl_set_mag_ready_callback
 * @param p - Pointer to an initialized lis3mdl_t struct
 * @param callback - Function to call when new magnetometer data is
 *                    available, or NULL to disable
 *
 * Registers a callback invoked by lis3mdl_loop() with the raw
 * magnetometer reading whenever STATUS_REG.ZYXDA is set. Stored in
 * p->mag_ready; NULL by default.
 */
void lis3mdl_set_mag_ready_callback(lis3mdl_t *p, void (*callback)(lis3mdl_t *p, int16_t x, int16_t y, int16_t z));

/**
 * lis3mdl_loop
 * @param p - Pointer to an initialized lis3mdl_t struct
 * @return HAL_OK on successful I2C access
 *
 * Service handler - reads STATUS_REG and, if ZYXDA is set, reads the
 * X/Y/Z axes and invokes the registered mag_ready callback (if any).
 * Intended to be called from the idle loop, gated by a flags.h flag
 * matching the configured ODR.
 */
HAL_StatusTypeDef lis3mdl_loop(lis3mdl_t *p);

/**
 * lis3mdl_compute_heading
 * @param mx - Magnetometer X reading - either raw (as from
 *             lis3mdl_read_mag()) or hard/soft-iron corrected (as from
 *             lis3mdl_apply_calibration())
 * @param my - Magnetometer Y reading, same units as mx
 * @param mz - Magnetometer Z reading, same units as mx
 * @param roll_deg - Roll angle in degrees, as computed by
 *                    lsm6dsrx_compute_tilt() for the paired IMU: rotation
 *                    about the shared X axis, positive when the +Y axis
 *                    moves towards +Z. The LIS3MDL and LSM6DSOX/LSM6DSRX
 *                    on the Adafruit breakout share the same X/Y/Z axis
 *                    frame, so the IMU's roll/pitch apply directly here.
 * @param pitch_deg - Pitch angle in degrees, as computed by
 *                     lsm6dsrx_compute_tilt() for the paired IMU:
 *                     rotation about the shared Y axis, positive when the
 *                     +X axis moves towards +Z.
 * @param out_heading_deg - Tilt-compensated heading in degrees, in the
 *                           range [0, 360), measured clockwise from
 *                           magnetic north when sighting along the +X
 *                           axis with the sensor level. Accuracy depends
 *                           on whether mx/my/mz have been hard/soft-iron
 *                           corrected; not corrected for magnetic
 *                           declination - that remains future work.
 *
 * Pure math helper, no I2C traffic. Uses roll_deg/pitch_deg to rotate the
 * mx/my/mz vector back to level before computing heading, so the result
 * is valid even when the staff is tilted.
 */
void lis3mdl_compute_heading(float mx, float my, float mz, float roll_deg, float pitch_deg, float *out_heading_deg);

/**
 * lis3mdl_calibration_init
 * @param cal - Pointer to the lis3mdl_calibration_t struct to reset
 *
 * Resets the running per-axis min/max to "no samples seen yet"
 * (min = INT16_MAX, max = INT16_MIN for each axis), so the first call to
 * lis3mdl_calibration_update() establishes the initial range.
 */
void lis3mdl_calibration_init(lis3mdl_calibration_t *cal);

/**
 * lis3mdl_calibration_update
 * @param cal - Pointer to an initialized lis3mdl_calibration_t struct
 * @param mx - Raw magnetometer X output (as from lis3mdl_read_mag)
 * @param my - Raw magnetometer Y output
 * @param mz - Raw magnetometer Z output
 *
 * Widens cal's per-axis min/max to include this sample. Call repeatedly
 * while the sensor is rotated through as many orientations as possible -
 * the more of the full sphere of orientations is covered, the more
 * accurate the resulting calibration.
 */
void lis3mdl_calibration_update(lis3mdl_calibration_t *cal, int16_t mx, int16_t my, int16_t mz);

/**
 * lis3mdl_apply_calibration
 * @param cal - Pointer to a lis3mdl_calibration_t populated via
 *              lis3mdl_calibration_update()
 * @param mx - Raw magnetometer X output (as from lis3mdl_read_mag)
 * @param my - Raw magnetometer Y output
 * @param mz - Raw magnetometer Z output
 * @param out_x - Corrected X, suitable for lis3mdl_compute_heading()
 * @param out_y - Corrected Y, suitable for lis3mdl_compute_heading()
 * @param out_z - Corrected Z, suitable for lis3mdl_compute_heading()
 *
 * Pure math helper, no I2C traffic. For each axis, subtracts the
 * midpoint of cal's min/max (hard-iron offset, recentres the reading
 * ellipsoid on the origin) and scales by the average per-axis range
 * divided by that axis's range (soft-iron correction, normalises the
 * ellipsoid towards a sphere). Until every axis's range exceeds
 * LIS3MDL_CALIBRATION_MIN_RANGE (i.e. before lis3mdl_calibration_update()
 * has seen enough rotation to distinguish real field variation from
 * sensor noise), mx/my/mz are passed through uncorrected rather than
 * dividing by a near-zero range and amplifying that noise.
 */
void lis3mdl_apply_calibration(const lis3mdl_calibration_t *cal, int16_t mx, int16_t my, int16_t mz, float *out_x, float *out_y, float *out_z);

#endif /* INC_LIS3MDL_H_ */
