

#include "ssd1309.h"
#include <string.h>

/* I2C control bytes (SSD1309 datasheet section 8.1.5) */
#define SSD1309_I2C_CONTROL_CMD  0x00
#define SSD1309_I2C_CONTROL_DATA 0x40
#define SSD1309_I2C_TIMEOUT_MS   100

/* Fundamental command set (datasheet section 9.1/10) */
#define SSD1309_CMD_DISPLAY_OFF           0xAE
#define SSD1309_CMD_DISPLAY_ON            0xAF
#define SSD1309_CMD_SET_CLOCK_DIV         0xD5
#define SSD1309_CMD_SET_MULTIPLEX         0xA8
#define SSD1309_CMD_SET_DISPLAY_OFFSET    0xD3
#define SSD1309_CMD_SET_START_LINE        0x40
#define SSD1309_CMD_SEGMENT_REMAP         0xA1
#define SSD1309_CMD_COM_SCAN_DEC          0xC8
#define SSD1309_CMD_SET_COM_PINS          0xDA
#define SSD1309_CMD_SET_CONTRAST          0x81
#define SSD1309_CMD_SET_PRECHARGE         0xD9
#define SSD1309_CMD_SET_VCOMH             0xDB
#define SSD1309_CMD_ENTIRE_DISPLAY_RESUME 0xA4
#define SSD1309_CMD_NORMAL_DISPLAY        0xA6
#define SSD1309_CMD_DEACTIVATE_SCROLL     0x2E

/* Addressing setting command set (datasheet section 9.3/10) */
#define SSD1309_CMD_SET_MEMORY_ADDR_MODE  0x20
#define SSD1309_MEMORY_ADDR_MODE_PAGE     0x02
#define SSD1309_CMD_SET_PAGE_START        0xB0
#define SSD1309_CMD_SET_LOW_COLUMN        0x00
#define SSD1309_CMD_SET_HIGH_COLUMN       0x10

static HAL_StatusTypeDef ssd1309_write_cmd(ssd1309_t *p, const uint8_t *cmds, uint16_t len)
{
	return HAL_I2C_Mem_Write(
		p->port,
		(uint16_t)(p->address << 1),
		SSD1309_I2C_CONTROL_CMD,
		I2C_MEMADD_SIZE_8BIT,
		(uint8_t *)cmds,
		len,
		SSD1309_I2C_TIMEOUT_MS
	);
}

static HAL_StatusTypeDef ssd1309_write_data(ssd1309_t *p, const uint8_t *data, uint16_t len)
{
	return HAL_I2C_Mem_Write(
		p->port,
		(uint16_t)(p->address << 1),
		SSD1309_I2C_CONTROL_DATA,
		I2C_MEMADD_SIZE_8BIT,
		(uint8_t *)data,
		len,
		SSD1309_I2C_TIMEOUT_MS
	);
}

void ssd1309_init(
	ssd1309_t *p,
	I2C_HandleTypeDef *in_port,
	uint16_t in_address,
	int16_t in_height,
	int16_t in_width)
{
	p->port = in_port;
	p->address = in_address;
	p->height = (in_height < 0) ? 64 : in_height;
	p->width = (in_width < 0) ? 128 : in_width;
}

HAL_StatusTypeDef ssd1309_bringup(ssd1309_t *p)
{
	HAL_StatusTypeDef status;

	uint8_t init_cmds[] = {
		SSD1309_CMD_DISPLAY_OFF,
		SSD1309_CMD_SET_CLOCK_DIV, 0x80,
		SSD1309_CMD_SET_MULTIPLEX, (uint8_t)(p->height - 1),
		SSD1309_CMD_SET_DISPLAY_OFFSET, 0x00,
		SSD1309_CMD_SET_START_LINE,
		SSD1309_CMD_SEGMENT_REMAP,
		SSD1309_CMD_COM_SCAN_DEC,
		SSD1309_CMD_SET_COM_PINS, 0x12,
		SSD1309_CMD_SET_CONTRAST, 0x8F,
		SSD1309_CMD_SET_PRECHARGE, 0xF1,
		SSD1309_CMD_SET_VCOMH, 0x40,
		SSD1309_CMD_ENTIRE_DISPLAY_RESUME,
		SSD1309_CMD_NORMAL_DISPLAY,
		SSD1309_CMD_DEACTIVATE_SCROLL,
	};

	status = ssd1309_write_cmd(p, init_cmds, sizeof(init_cmds));
	if (status != HAL_OK) {
		return status;
	}

	uint8_t mem_addr_mode[] = {
		SSD1309_CMD_SET_MEMORY_ADDR_MODE, SSD1309_MEMORY_ADDR_MODE_PAGE,
	};
	status = ssd1309_write_cmd(p, mem_addr_mode, sizeof(mem_addr_mode));
	if (status != HAL_OK) {
		return status;
	}

	uint8_t page_buf[128];
	memset(page_buf, 0xFF, sizeof(page_buf));

	uint8_t num_pages = (uint8_t)(p->height / 8);
	for (uint8_t page = 0; page < num_pages; page++) {
		uint8_t page_cmds[] = {
			(uint8_t)(SSD1309_CMD_SET_PAGE_START + page),
			SSD1309_CMD_SET_LOW_COLUMN,
			SSD1309_CMD_SET_HIGH_COLUMN,
		};
		status = ssd1309_write_cmd(p, page_cmds, sizeof(page_cmds));
		if (status != HAL_OK) {
			return status;
		}

		status = ssd1309_write_data(p, page_buf, (uint16_t)p->width);
		if (status != HAL_OK) {
			return status;
		}
	}

	uint8_t display_on[] = { SSD1309_CMD_DISPLAY_ON };
	return ssd1309_write_cmd(p, display_on, sizeof(display_on));
}
