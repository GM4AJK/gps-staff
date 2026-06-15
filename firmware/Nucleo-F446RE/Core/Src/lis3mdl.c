

#include "lis3mdl.h"
#include "app.h"
#include <math.h>

#define LIS3MDL_I2C_TIMEOUT_MS 100

#define LIS3MDL_DEG_TO_RAD (3.14159265358979323846f / 180.0f)
#define LIS3MDL_RAD_TO_DEG (180.0f / 3.14159265358979323846f)

/* Below this per-axis range (LSB, +/-4 gauss FS), an axis's min/max is
 * indistinguishable from noise - applying the soft-iron scale would
 * divide by near-zero and amplify that noise into a wildly unstable
 * heading. Until every axis exceeds this, lis3mdl_apply_calibration()
 * returns the raw reading uncorrected. */
#define LIS3MDL_CALIBRATION_MIN_RANGE 50.0f

/* CTRL_REG1: TEMP_EN=0, OM=11 (X/Y ultrahigh-performance), DO=100 (10Hz) */
#define LIS3MDL_CTRL_REG1_UHP_10HZ 0x70

/* CTRL_REG2: FS=00 (+/-4 gauss, reset default) */
#define LIS3MDL_CTRL_REG2_4GAUSS 0x00

/* CTRL_REG3: MD=00 (continuous-conversion mode) */
#define LIS3MDL_CTRL_REG3_CONTINUOUS 0x00

/* CTRL_REG4: OMZ=11 (Z ultrahigh-performance) */
#define LIS3MDL_CTRL_REG4_UHP 0x0C

static HAL_StatusTypeDef lis3mdl_read_reg(lis3mdl_t *p, uint8_t reg, uint8_t *data, uint16_t len)
{
	return HAL_I2C_Mem_Read(
		p->port,
		(uint16_t)(p->address << 1),
		reg,
		I2C_MEMADD_SIZE_8BIT,
		data,
		len,
		LIS3MDL_I2C_TIMEOUT_MS
	);
}

static HAL_StatusTypeDef lis3mdl_write_reg(lis3mdl_t *p, uint8_t reg, uint8_t value)
{
	return HAL_I2C_Mem_Write(
		p->port,
		(uint16_t)(p->address << 1),
		reg,
		I2C_MEMADD_SIZE_8BIT,
		&value,
		1,
		LIS3MDL_I2C_TIMEOUT_MS
	);
}

void lis3mdl_init(lis3mdl_t *p, I2C_HandleTypeDef *in_port, uint16_t in_address)
{
	p->port = in_port;
	p->address = in_address;
	p->mag_ready = NULL;
}

HAL_StatusTypeDef lis3mdl_bringup(lis3mdl_t *p)
{
	HAL_StatusTypeDef status;
	uint8_t who_am_i;

	status = lis3mdl_read_reg(p, LIS3MDL_REG_WHO_AM_I, &who_am_i, 1);
	if (status != HAL_OK || who_am_i != LIS3MDL_WHOAMI) {
		p->address = LIS3MDL_I2C_ADDR_SA1_HIGH;
		status = lis3mdl_read_reg(p, LIS3MDL_REG_WHO_AM_I, &who_am_i, 1);
		if (status != HAL_OK || who_am_i != LIS3MDL_WHOAMI) {
			app_log("lis3mdl: who_am_i mismatch (0x%02X)\r\n", who_am_i);
			return HAL_ERROR;
		}
	}

	app_log("lis3mdl: found LIS3MDL at 0x%02X\r\n", p->address);

	status = lis3mdl_write_reg(p, LIS3MDL_REG_CTRL_REG1, LIS3MDL_CTRL_REG1_UHP_10HZ);
	if (status != HAL_OK) {
		return status;
	}

	status = lis3mdl_write_reg(p, LIS3MDL_REG_CTRL_REG2, LIS3MDL_CTRL_REG2_4GAUSS);
	if (status != HAL_OK) {
		return status;
	}

	status = lis3mdl_write_reg(p, LIS3MDL_REG_CTRL_REG3, LIS3MDL_CTRL_REG3_CONTINUOUS);
	if (status != HAL_OK) {
		return status;
	}

	return lis3mdl_write_reg(p, LIS3MDL_REG_CTRL_REG4, LIS3MDL_CTRL_REG4_UHP);
}

HAL_StatusTypeDef lis3mdl_data_ready(lis3mdl_t *p, bool *out_ready)
{
	uint8_t status_reg;

	HAL_StatusTypeDef status = lis3mdl_read_reg(p, LIS3MDL_REG_STATUS_REG, &status_reg, 1);
	if (status != HAL_OK) {
		return status;
	}

	*out_ready = (status_reg & 0x08) != 0;

	return HAL_OK;
}

HAL_StatusTypeDef lis3mdl_read_mag(lis3mdl_t *p, int16_t *out_x, int16_t *out_y, int16_t *out_z)
{
	uint8_t data[6];

	HAL_StatusTypeDef status = lis3mdl_read_reg(p, LIS3MDL_REG_OUT_X_L | LIS3MDL_AUTO_INCREMENT, data, sizeof(data));
	if (status != HAL_OK) {
		return status;
	}

	*out_x = (int16_t)((data[1] << 8) | data[0]);
	*out_y = (int16_t)((data[3] << 8) | data[2]);
	*out_z = (int16_t)((data[5] << 8) | data[4]);

	return HAL_OK;
}

void lis3mdl_set_mag_ready_callback(lis3mdl_t *p, void (*callback)(lis3mdl_t *p, int16_t x, int16_t y, int16_t z))
{
	p->mag_ready = callback;
}

void lis3mdl_compute_heading(float mx, float my, float mz, float roll_deg, float pitch_deg, float *out_heading_deg)
{
	float roll_rad = roll_deg * LIS3MDL_DEG_TO_RAD;
	float pitch_rad = pitch_deg * LIS3MDL_DEG_TO_RAD;

	float xh = mx * cosf(pitch_rad) + mz * sinf(pitch_rad);
	float yh = mx * sinf(roll_rad) * sinf(pitch_rad)
		+ my * cosf(roll_rad)
		- mz * sinf(roll_rad) * cosf(pitch_rad);

	float heading_deg = atan2f(yh, xh) * LIS3MDL_RAD_TO_DEG;
	if (heading_deg < 0.0f) {
		heading_deg += 360.0f;
	}

	*out_heading_deg = heading_deg;
}

void lis3mdl_calibration_init(lis3mdl_calibration_t *cal)
{
	cal->min_x = cal->min_y = cal->min_z = INT16_MAX;
	cal->max_x = cal->max_y = cal->max_z = INT16_MIN;
}

void lis3mdl_calibration_update(lis3mdl_calibration_t *cal, int16_t mx, int16_t my, int16_t mz)
{
	if (mx < cal->min_x) cal->min_x = mx;
	if (mx > cal->max_x) cal->max_x = mx;
	if (my < cal->min_y) cal->min_y = my;
	if (my > cal->max_y) cal->max_y = my;
	if (mz < cal->min_z) cal->min_z = mz;
	if (mz > cal->max_z) cal->max_z = mz;
}

void lis3mdl_apply_calibration(const lis3mdl_calibration_t *cal, int16_t mx, int16_t my, int16_t mz, float *out_x, float *out_y, float *out_z)
{
	float offset_x = (float)(cal->max_x + cal->min_x) / 2.0f;
	float offset_y = (float)(cal->max_y + cal->min_y) / 2.0f;
	float offset_z = (float)(cal->max_z + cal->min_z) / 2.0f;

	float range_x = (float)(cal->max_x - cal->min_x) / 2.0f;
	float range_y = (float)(cal->max_y - cal->min_y) / 2.0f;
	float range_z = (float)(cal->max_z - cal->min_z) / 2.0f;

	if (range_x < LIS3MDL_CALIBRATION_MIN_RANGE || range_y < LIS3MDL_CALIBRATION_MIN_RANGE || range_z < LIS3MDL_CALIBRATION_MIN_RANGE) {
		*out_x = (float)mx;
		*out_y = (float)my;
		*out_z = (float)mz;
		return;
	}

	float avg_range = (range_x + range_y + range_z) / 3.0f;

	*out_x = ((float)mx - offset_x) * (avg_range / range_x);
	*out_y = ((float)my - offset_y) * (avg_range / range_y);
	*out_z = ((float)mz - offset_z) * (avg_range / range_z);
}

HAL_StatusTypeDef lis3mdl_loop(lis3mdl_t *p)
{
	bool ready;
	int16_t x, y, z;

	HAL_StatusTypeDef status = lis3mdl_data_ready(p, &ready);
	if (status != HAL_OK) {
		return status;
	}

	if (ready && p->mag_ready != NULL) {
		status = lis3mdl_read_mag(p, &x, &y, &z);
		if (status != HAL_OK) {
			return status;
		}
		p->mag_ready(p, x, y, z);
	}

	return HAL_OK;
}
