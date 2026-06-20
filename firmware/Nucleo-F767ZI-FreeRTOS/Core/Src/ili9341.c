#include "ili9341.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <string.h>

#define SPI_TIMEOUT HAL_MAX_DELAY

/* Binary semaphore signalled from HAL_SPI_TxCpltCallback via ili9341_spi_tx_cplt() */
static SemaphoreHandle_t spi_tx_done;

/* Rows sent per DMA transfer. 32 rows × 480 bytes = 15 KB; 10 transfers per full fill. */
#define DMA_ROWS 32
static uint8_t dma_buf[ILI9341_WIDTH * DMA_ROWS * 2] __attribute__((aligned(32)));

void ili9341_dma_init(void)
{
	spi_tx_done = xSemaphoreCreateBinary();
	configASSERT(spi_tx_done);
}

void ili9341_spi_tx_cplt(void)
{
	BaseType_t woken = pdFALSE;
	xSemaphoreGiveFromISR(spi_tx_done, &woken);
	portYIELD_FROM_ISR(woken);
}

/* Marker byte in init table meaning "delay N ms" instead of a command */
#define DELAY 0xFF

static void cs(ili9341_t *p, uint8_t lvl)
{
	HAL_GPIO_WritePin(p->cs_port, p->cs_pin,
	                  lvl ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void dc(ili9341_t *p, uint8_t lvl)
{
	HAL_GPIO_WritePin(p->dc_port, p->dc_pin,
	                  lvl ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void write_cmd(ili9341_t *p, uint8_t cmd)
{
	dc(p, 0);
	cs(p, 0);
	HAL_SPI_Transmit(p->spi, &cmd, 1, SPI_TIMEOUT);
	cs(p, 1);
}

static void write_data(ili9341_t *p, const uint8_t *buf, size_t len)
{
	dc(p, 1);
	cs(p, 0);
	HAL_SPI_Transmit(p->spi, (uint8_t *)buf, len, SPI_TIMEOUT);
	cs(p, 1);
}

static void set_window(ili9341_t *p,
                       uint16_t x0, uint16_t y0,
                       uint16_t x1, uint16_t y1)
{
	uint8_t buf[4];

	write_cmd(p, 0x2A);
	buf[0] = x0 >> 8; buf[1] = x0 & 0xFF;
	buf[2] = x1 >> 8; buf[3] = x1 & 0xFF;
	write_data(p, buf, 4);

	write_cmd(p, 0x2B);
	buf[0] = y0 >> 8; buf[1] = y0 & 0xFF;
	buf[2] = y1 >> 8; buf[3] = y1 & 0xFF;
	write_data(p, buf, 4);

	write_cmd(p, 0x2C);
}

/*
 * Init table: {cmd, n_data, data[0..n-1]}
 * cmd == DELAY means n_data is the delay in ms, no data bytes follow.
 *
 * Minimal sequence using only documented ILI9341 registers. Extended
 * panel-specific commands (0xCB, 0xCF, 0xE8, 0xEA, 0xED, 0xF7) are
 * omitted — they vary by panel and are not required for basic operation.
 */
static const uint8_t init_seq[] = {
	0x01, 0,                /* Software reset */
	DELAY, 150,
	0xC0, 1, 0x23,          /* Power Control 1: VRH=4.60V */
	0xC1, 1, 0x10,          /* Power Control 2: SAP, BT */
	0xC5, 2, 0x3E, 0x28,    /* VCOM Control 1 */
	0xC7, 1, 0x86,          /* VCOM Control 2 */
	0x36, 1, 0x48,          /* MADCTL: portrait, BGR */
	0x3A, 1, 0x55,          /* Pixel format: 16-bit RGB565 */
	0xB1, 2, 0x00, 0x18,    /* Frame rate: ~79 Hz */
	0xB6, 3, 0x08, 0x82, 0x27, /* Display function control */
	0x26, 1, 0x01,          /* Gamma curve 1 */
	0xE0, 15,               /* Positive gamma */
		0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08,
		0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03,
		0x0E, 0x09, 0x00,
	0xE1, 15,               /* Negative gamma */
		0x00, 0x0E, 0x14, 0x03, 0x11, 0x07,
		0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C,
		0x31, 0x36, 0x0F,
	0x11, 0,                /* Sleep out */
	DELAY, 120,
	0x29, 0,                /* Display on */
};

void ili9341_init(ili9341_t *p,
	SPI_HandleTypeDef *spi,
	GPIO_TypeDef *cs_port,  uint16_t cs_pin,
	GPIO_TypeDef *dc_port,  uint16_t dc_pin,
	GPIO_TypeDef *rst_port, uint16_t rst_pin,
	GPIO_TypeDef *bl_port,  uint16_t bl_pin)
{
	p->spi      = spi;
	p->cs_port  = cs_port;   p->cs_pin  = cs_pin;
	p->dc_port  = dc_port;   p->dc_pin  = dc_pin;
	p->rst_port = rst_port;  p->rst_pin = rst_pin;
	p->bl_port  = bl_port;   p->bl_pin  = bl_pin;
	p->width    = ILI9341_WIDTH;
	p->height   = ILI9341_HEIGHT;

	cs(p, 1);
	dc(p, 1);
	HAL_GPIO_WritePin(rst_port, rst_pin, GPIO_PIN_SET);
}

void ili9341_bringup(ili9341_t *p)
{
	/* Hardware reset */
	HAL_GPIO_WritePin(p->rst_port, p->rst_pin, GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(p->rst_port, p->rst_pin, GPIO_PIN_SET);
	HAL_Delay(120);

	/* Walk the init table */
	const uint8_t *t = init_seq;
	const uint8_t *end = init_seq + sizeof(init_seq);

	while (t < end) {
		uint8_t cmd = *t++;
		uint8_t n   = *t++;

		if (cmd == DELAY) {
			HAL_Delay(n);
		} else {
			write_cmd(p, cmd);
			if (n > 0)
				write_data(p, t, n);
			t += n;
		}
	}
}

void ili9341_backlight(ili9341_t *p, uint8_t on)
{
	HAL_GPIO_WritePin(p->bl_port, p->bl_pin,
	                  on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void ili9341_fill(ili9341_t *p, uint16_t colour)
{
	ili9341_fill_rect(p, 0, 0, p->width, p->height, colour);
}

void ili9341_fill_rect(ili9341_t *p,
                       uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                       uint16_t colour)
{
	if (w == 0 || h == 0) return;

	set_window(p, x, y, x + w - 1, y + h - 1);

	uint8_t hi = colour >> 8;
	uint8_t lo = colour & 0xFF;
	for (int i = 0; i < (int)sizeof(dma_buf); i += 2) {
		dma_buf[i]   = hi;
		dma_buf[i+1] = lo;
	}
	/* One cache flush covers all transfers — dma_buf is not touched between them */
	SCB_CleanDCache_by_Addr((uint32_t *)dma_buf, sizeof(dma_buf));

	dc(p, 1);
	cs(p, 0);
	uint32_t rows_left = h;
	while (rows_left > 0) {
		uint16_t rows = (rows_left > DMA_ROWS) ? DMA_ROWS : (uint16_t)rows_left;
		HAL_SPI_Transmit_DMA(p->spi, dma_buf, (uint16_t)(rows * w * 2u));
		xSemaphoreTake(spi_tx_done, pdMS_TO_TICKS(100));
		rows_left -= rows;
	}
	cs(p, 1);
}

void ili9341_draw_pixel(ili9341_t *p, uint16_t x, uint16_t y, uint16_t colour)
{
	ili9341_fill_rect(p, x, y, 1, 1, colour);
}

void ili9341_read_id(ili9341_t *p, uint8_t id[3])
{
	uint8_t buf[4] = {0, 0, 0, 0};

	dc(p, 0);
	cs(p, 0);
	uint8_t cmd = 0x04;
	HAL_SPI_Transmit(p->spi, &cmd, 1, SPI_TIMEOUT);
	dc(p, 1);
	HAL_SPI_Receive(p->spi, buf, 4, SPI_TIMEOUT);
	cs(p, 1);

	/* buf[0] is a dummy byte; buf[1..3] are the three ID bytes */
	id[0] = buf[1];
	id[1] = buf[2];
	id[2] = buf[3];
}

void ili9341_draw_image(ili9341_t *p,
                        uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                        const uint8_t *data)
{
	set_window(p, x, y, x + w - 1, y + h - 1);
	dc(p, 1);
	cs(p, 0);

	/* DMA cannot source directly from Flash on Cortex-M7 AXIM bus.
	 * Copy each chunk into dma_buf (SRAM) first, then DMA from there. */
	uint32_t remaining = (uint32_t)w * h * 2;
	const uint8_t *src = data;
	while (remaining > 0) {
		uint16_t chunk = (remaining > sizeof(dma_buf)) ? (uint16_t)sizeof(dma_buf) : (uint16_t)remaining;
		memcpy(dma_buf, src, chunk);
		SCB_CleanDCache_by_Addr((uint32_t *)dma_buf, sizeof(dma_buf));
		HAL_SPI_Transmit_DMA(p->spi, dma_buf, chunk);
		xSemaphoreTake(spi_tx_done, pdMS_TO_TICKS(100));
		src       += chunk;
		remaining -= chunk;
	}

	cs(p, 1);
}

void ili9341_write_pixels(ili9341_t *p,
                          uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                          const uint16_t *pixels, uint32_t n_pixels)
{
	set_window(p, x, y, x + w - 1, y + h - 1);

	dc(p, 1);
	cs(p, 0);
	for (uint32_t i = 0; i < n_pixels; i++) {
		uint8_t pair[2] = { pixels[i] >> 8, pixels[i] & 0xFF };
		HAL_SPI_Transmit(p->spi, pair, 2, SPI_TIMEOUT);
	}
	cs(p, 1);
}
