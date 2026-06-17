
#include "config.h"
#include "app.h"
#include <string.h>
#include <stddef.h>

_Static_assert(sizeof(config_data_t) == 64, "config_data_t must be exactly 64 bytes");

/* AT24C256 internal address of the config page */
#define EEPROM_PAGE_ADDR     0x0000
/* Attempts to poll for write-cycle completion (1 ms apart, 10 ms max) */
#define WRITE_POLL_ATTEMPTS  10

static uint16_t crc16(const uint8_t *buf, size_t len)
{
	uint16_t crc = 0xFFFF;
	for(size_t i = 0; i < len; i++) {
		crc ^= (uint16_t)buf[i] << 8;
		for(int b = 0; b < 8; b++)
			crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
	}
	return crc;
}

static uint16_t data_crc(const config_data_t *d)
{
	return crc16((const uint8_t *)d, offsetof(config_data_t, crc));
}

static void load_defaults(config_data_t *d)
{
	memset(d, 0, sizeof(*d));
	d->magic   = CONFIG_MAGIC;
	d->version = CONFIG_VERSION;
	d->soft_iron[0][0] = 1.0f;
	d->soft_iron[1][1] = 1.0f;
	d->soft_iron[2][2] = 1.0f;
	d->crc = data_crc(d);
}

static HAL_StatusTypeDef eeprom_write(config_t *p)
{
	p->data.crc = data_crc(&p->data);
	HAL_StatusTypeDef r = HAL_I2C_Mem_Write(
		p->port, p->i2c_addr, EEPROM_PAGE_ADDR, I2C_MEMADD_SIZE_16BIT,
		(uint8_t *)&p->data, sizeof(p->data), 100);
	if(r != HAL_OK)
		return r;
	for(int i = 0; i < WRITE_POLL_ATTEMPTS; i++) {
		HAL_Delay(1);
		if(HAL_I2C_IsDeviceReady(p->port, p->i2c_addr, 1, 10) == HAL_OK)
			return HAL_OK;
	}
	return HAL_TIMEOUT;
}

void config_init(config_t *p, I2C_HandleTypeDef *port, uint16_t i2c_addr)
{
	p->port     = port;
	p->i2c_addr = i2c_addr;
	load_defaults(&p->data);
}

HAL_StatusTypeDef config_load(config_t *p)
{
	config_data_t tmp;
	HAL_StatusTypeDef r = HAL_I2C_Mem_Read(
		p->port, p->i2c_addr, EEPROM_PAGE_ADDR, I2C_MEMADD_SIZE_16BIT,
		(uint8_t *)&tmp, sizeof(tmp), 100);
	if(r != HAL_OK) {
		app_log("config: EEPROM read failed (%d), using defaults\r\n", r);
		load_defaults(&p->data);
		return HAL_ERROR;
	}
	if(tmp.magic != CONFIG_MAGIC || tmp.crc != data_crc(&tmp)) {
		app_log("config: invalid magic/CRC, using defaults\r\n");
		load_defaults(&p->data);
		return HAL_ERROR;
	}
	p->data = tmp;
	app_log("config: loaded v%u\r\n", p->data.version);
	return HAL_OK;
}

void config_get_mag_hard_iron(const config_t *p, float *out_x, float *out_y, float *out_z)
{
	*out_x = p->data.hard_iron[0];
	*out_y = p->data.hard_iron[1];
	*out_z = p->data.hard_iron[2];
}

void config_get_mag_soft_iron(const config_t *p, float out_matrix[3][3])
{
	memcpy(out_matrix, p->data.soft_iron, sizeof(p->data.soft_iron));
}

HAL_StatusTypeDef config_set_mag_hard_iron(config_t *p, float x, float y, float z)
{
	p->data.hard_iron[0] = x;
	p->data.hard_iron[1] = y;
	p->data.hard_iron[2] = z;
	return eeprom_write(p);
}

HAL_StatusTypeDef config_set_mag_soft_iron(config_t *p, const float matrix[3][3])
{
	memcpy(p->data.soft_iron, matrix, sizeof(p->data.soft_iron));
	return eeprom_write(p);
}
