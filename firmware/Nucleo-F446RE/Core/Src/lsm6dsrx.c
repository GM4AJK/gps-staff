

#include "lsm6dsrx.h"
#include "app.h"
#include <math.h>

#define LSM6DSRX_I2C_TIMEOUT_MS 100

#define LSM6DSRX_RAD_TO_DEG (180.0f / 3.14159265358979323846f)

/* CTRL1_XL / CTRL2_G: ODR=104Hz (high-perf, 0100), FS=00 (+/-2g / +/-250dps) */
#define LSM6DSRX_CTRL1_XL_104HZ_2G    0x40
#define LSM6DSRX_CTRL2_G_104HZ_250DPS 0x40

/* CTRL3_C: BDU=1, IF_INC=1 (IF_INC is already the reset default) */
#define LSM6DSRX_CTRL3_C_BDU_IFINC 0x44

static HAL_StatusTypeDef lsm6dsrx_read_reg(lsm6dsrx_t *p, uint8_t reg, uint8_t *data, uint16_t len)
{
	return HAL_I2C_Mem_Read(
		p->port,
		(uint16_t)(p->address << 1),
		reg,
		I2C_MEMADD_SIZE_8BIT,
		data,
		len,
		LSM6DSRX_I2C_TIMEOUT_MS
	);
}

static HAL_StatusTypeDef lsm6dsrx_write_reg(lsm6dsrx_t *p, uint8_t reg, uint8_t value)
{
	return HAL_I2C_Mem_Write(
		p->port,
		(uint16_t)(p->address << 1),
		reg,
		I2C_MEMADD_SIZE_8BIT,
		&value,
		1,
		LSM6DSRX_I2C_TIMEOUT_MS
	);
}

static HAL_StatusTypeDef lsm6dsrx_read_axes(lsm6dsrx_t *p, uint8_t reg, int16_t *out_x, int16_t *out_y, int16_t *out_z)
{
	uint8_t data[6];

	HAL_StatusTypeDef status = lsm6dsrx_read_reg(p, reg, data, sizeof(data));
	if (status != HAL_OK) {
		return status;
	}

	*out_x = (int16_t)((data[1] << 8) | data[0]);
	*out_y = (int16_t)((data[3] << 8) | data[2]);
	*out_z = (int16_t)((data[5] << 8) | data[4]);

	return HAL_OK;
}

void lsm6dsrx_init(lsm6dsrx_t *p, I2C_HandleTypeDef *in_port, uint16_t in_address)
{
	p->port = in_port;
	p->address = in_address;
	p->accel_ready = NULL;
	p->gyro_ready = NULL;
}

HAL_StatusTypeDef lsm6dsrx_bringup(lsm6dsrx_t *p)
{
	HAL_StatusTypeDef status;
	uint8_t who_am_i;

	status = lsm6dsrx_read_reg(p, LSM6DSRX_REG_WHO_AM_I, &who_am_i, 1);
	if (status != HAL_OK || (who_am_i != LSM6DSRX_WHOAMI_LSM6DSRX && who_am_i != LSM6DSRX_WHOAMI_LSM6DSOX)) {
		p->address = LSM6DSRX_I2C_ADDR_SA0_HIGH;
		status = lsm6dsrx_read_reg(p, LSM6DSRX_REG_WHO_AM_I, &who_am_i, 1);
		if (status != HAL_OK || (who_am_i != LSM6DSRX_WHOAMI_LSM6DSRX && who_am_i != LSM6DSRX_WHOAMI_LSM6DSOX)) {
			app_log("lsm6dsrx: who_am_i mismatch (0x%02X)\r\n", who_am_i);
			return HAL_ERROR;
		}
	}

	app_log(
		"lsm6dsrx: found %s at 0x%02X\r\n",
		(who_am_i == LSM6DSRX_WHOAMI_LSM6DSRX) ? "LSM6DSRX" : "LSM6DSOX",
		p->address
	);

	status = lsm6dsrx_write_reg(p, LSM6DSRX_REG_CTRL3_C, LSM6DSRX_CTRL3_C_BDU_IFINC);
	if (status != HAL_OK) {
		return status;
	}

	status = lsm6dsrx_write_reg(p, LSM6DSRX_REG_CTRL1_XL, LSM6DSRX_CTRL1_XL_104HZ_2G);
	if (status != HAL_OK) {
		return status;
	}

	return lsm6dsrx_write_reg(p, LSM6DSRX_REG_CTRL2_G, LSM6DSRX_CTRL2_G_104HZ_250DPS);
}

HAL_StatusTypeDef lsm6dsrx_data_ready(lsm6dsrx_t *p, bool *out_xl_ready, bool *out_g_ready)
{
	uint8_t status_reg;

	HAL_StatusTypeDef status = lsm6dsrx_read_reg(p, LSM6DSRX_REG_STATUS_REG, &status_reg, 1);
	if (status != HAL_OK) {
		return status;
	}

	*out_xl_ready = (status_reg & 0x01) != 0;
	*out_g_ready = (status_reg & 0x02) != 0;

	return HAL_OK;
}

HAL_StatusTypeDef lsm6dsrx_read_accel(lsm6dsrx_t *p, int16_t *out_x, int16_t *out_y, int16_t *out_z)
{
	return lsm6dsrx_read_axes(p, LSM6DSRX_REG_OUTX_L_A, out_x, out_y, out_z);
}

HAL_StatusTypeDef lsm6dsrx_read_gyro(lsm6dsrx_t *p, int16_t *out_x, int16_t *out_y, int16_t *out_z)
{
	return lsm6dsrx_read_axes(p, LSM6DSRX_REG_OUTX_L_G, out_x, out_y, out_z);
}

void lsm6dsrx_set_accel_ready_callback(lsm6dsrx_t *p, void (*callback)(lsm6dsrx_t *p, int16_t x, int16_t y, int16_t z))
{
	p->accel_ready = callback;
}

void lsm6dsrx_set_gyro_ready_callback(lsm6dsrx_t *p, void (*callback)(lsm6dsrx_t *p, int16_t x, int16_t y, int16_t z))
{
	p->gyro_ready = callback;
}

void lsm6dsrx_compute_tilt(int16_t ax, int16_t ay, int16_t az, float *out_roll_deg, float *out_pitch_deg)
{
	*out_roll_deg = atan2f((float)ay, (float)az) * LSM6DSRX_RAD_TO_DEG;
	*out_pitch_deg = atan2f(-(float)ax, sqrtf((float)ay * (float)ay + (float)az * (float)az)) * LSM6DSRX_RAD_TO_DEG;
}

HAL_StatusTypeDef lsm6dsrx_loop(lsm6dsrx_t *p)
{
	bool xl_ready, g_ready;
	int16_t x, y, z;

	HAL_StatusTypeDef status = lsm6dsrx_data_ready(p, &xl_ready, &g_ready);
	if (status != HAL_OK) {
		return status;
	}

	if (xl_ready && p->accel_ready != NULL) {
		status = lsm6dsrx_read_accel(p, &x, &y, &z);
		if (status != HAL_OK) {
			return status;
		}
		p->accel_ready(p, x, y, z);
	}

	if (g_ready && p->gyro_ready != NULL) {
		status = lsm6dsrx_read_gyro(p, &x, &y, &z);
		if (status != HAL_OK) {
			return status;
		}
		p->gyro_ready(p, x, y, z);
	}

	return HAL_OK;
}
