#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "main.h"
#include "rtcm3.h"

#define RTCM3_PREAMBLE  0xD3

static UART_HandleTypeDef *uart_f9p;
static UART_HandleTypeDef *uart_dbg;

static uint8_t bufs[RTCM3_BUF_COUNT][RTCM3_BUF_SIZE];
static uint16_t buf_len[RTCM3_BUF_COUNT];

static volatile int in_buf_idx;
static int out_buf_idx;

/* Bitmask: bit N set means buffer N holds a complete, unprocessed frame */
static volatile uint8_t ready_mask;

/* Single-byte staging buffer used by the ISR */
static uint8_t irq_rx_byte;

typedef enum {
	ST_SEARCH,
	ST_LEN1,
	ST_LEN2,
	ST_DATA,
} irq_state_t;

static volatile irq_state_t irq_state;
static volatile uint16_t irq_bytes_remaining;
static volatile uint16_t irq_buf_pos;

static uint32_t crc24q(const uint8_t *buf, int len)
{
	uint32_t crc = 0;
	while (len--) {
		crc ^= ((uint32_t)(*buf++) << 16);
		for (int i = 0; i < 8; i++) {
			crc <<= 1;
			if (crc & 0x1000000) crc ^= 0x1864CFB;
		}
	}
	return crc & 0xFFFFFF;
}

static int rtcm3_get_out_idx(void)
{
	if (ready_mask & (1u << out_buf_idx)) return out_buf_idx;
	return -1;
}

void rtcm3_init(UART_HandleTypeDef *f9p_uart, UART_HandleTypeDef *debug_uart)
{
	uart_f9p = f9p_uart;
	uart_dbg = debug_uart;

	memset(bufs, 0, sizeof(bufs));
	memset(buf_len, 0, sizeof(buf_len));
	in_buf_idx = 0;
	out_buf_idx = 0;
	ready_mask = 0;
	irq_state = ST_SEARCH;
	irq_buf_pos = 0;
	irq_bytes_remaining = 0;

	HAL_UART_Receive_IT(uart_f9p, &irq_rx_byte, 1);
}

void rtcm3_uart_in_irq(void)
{
	uint8_t byte = irq_rx_byte;
	HAL_UART_Receive(uart_f9p, &irq_rx_byte, 1, 1);

	switch (irq_state) {
	case ST_SEARCH:
		if (byte == RTCM3_PREAMBLE) {
			if (ready_mask & (1u << in_buf_idx)) {
				break; /* all buffers full, drop frame */
			}
			bufs[in_buf_idx][0] = byte;
			irq_buf_pos = 1;
			irq_state = ST_LEN1;
		}
		break;

	case ST_LEN1:
		bufs[in_buf_idx][irq_buf_pos++] = byte;
		irq_bytes_remaining = (uint16_t)(byte & 0x03) << 8;
		irq_state = ST_LEN2;
		break;

	case ST_LEN2:
		bufs[in_buf_idx][irq_buf_pos++] = byte;
		irq_bytes_remaining |= byte;
		irq_bytes_remaining += 3; /* account for 3 CRC bytes */
		irq_state = ST_DATA;
		break;

	case ST_DATA:
		if (irq_buf_pos < RTCM3_BUF_SIZE) {
			bufs[in_buf_idx][irq_buf_pos++] = byte;
		}
		if (--irq_bytes_remaining == 0) {
			buf_len[in_buf_idx] = irq_buf_pos;
			ready_mask |= (1u << in_buf_idx);
			in_buf_idx = (in_buf_idx + 1) % RTCM3_BUF_COUNT;
			irq_state = ST_SEARCH;
			irq_buf_pos = 0;
		}
		break;
	}
}

void rtcm3_loop(void)
{
	int idx = rtcm3_get_out_idx();
	if (idx < 0) return;

	uint8_t *frame = bufs[idx];
	uint16_t payload_len = ((uint16_t)(frame[1] & 0x03) << 8) | frame[2];

	uint32_t calc_crc = crc24q(frame, 3 + payload_len);
	uint32_t recv_crc = ((uint32_t)frame[3 + payload_len]     << 16)
	                  | ((uint32_t)frame[3 + payload_len + 1] <<  8)
	                  |  (uint32_t)frame[3 + payload_len + 2];

	char log[64];
	int len = snprintf(log, sizeof(log), "RTCM3 message received, CRC %s\r\n",
	                   (calc_crc == recv_crc) ? "OK" : "FAIL");
	HAL_UART_Transmit(uart_dbg, (uint8_t *)log, len, 100);

	ready_mask &= ~(1u << idx);
	out_buf_idx = (out_buf_idx + 1) % RTCM3_BUF_COUNT;
}
