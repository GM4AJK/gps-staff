#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "rtcm3.h"

#define RTCM3_PREAMBLE  0xD3

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

void rtcm3_init(rtcm3_t *p, UART_HandleTypeDef *f9p_uart,
                void (*on_frame)(const uint8_t *frame, uint16_t len))
{
	p->uart_f9p  = f9p_uart;
	p->on_frame  = on_frame;

	memset(p->bufs, 0, sizeof(p->bufs));
	memset(p->buf_len, 0, sizeof(p->buf_len));
	p->in_buf_idx        = 0;
	p->out_buf_idx       = 0;
	p->ready_mask        = 0;
	p->irq_state         = RTCM3_ST_SEARCH;
	p->irq_buf_pos       = 0;
	p->irq_bytes_remaining = 0;

	HAL_UART_Receive_IT(p->uart_f9p, &p->irq_rx_byte, 1);
}

void rtcm3_uart_in_irq(rtcm3_t *p)
{
	uint8_t byte = p->irq_rx_byte;
	HAL_UART_Receive(p->uart_f9p, &p->irq_rx_byte, 1, 1);

	switch (p->irq_state) {
	case RTCM3_ST_SEARCH:
		if (byte == RTCM3_PREAMBLE) {
			if (p->ready_mask & (1u << p->in_buf_idx)) {
				break; /* all buffers full, drop frame */
			}
			p->bufs[p->in_buf_idx][0] = byte;
			p->irq_buf_pos = 1;
			p->irq_state = RTCM3_ST_LEN1;
		}
		break;

	case RTCM3_ST_LEN1:
		p->bufs[p->in_buf_idx][p->irq_buf_pos++] = byte;
		p->irq_bytes_remaining = (uint16_t)(byte & 0x03) << 8;
		p->irq_state = RTCM3_ST_LEN2;
		break;

	case RTCM3_ST_LEN2:
		p->bufs[p->in_buf_idx][p->irq_buf_pos++] = byte;
		p->irq_bytes_remaining |= byte;
		p->irq_bytes_remaining += 3; /* account for 3 CRC bytes */
		p->irq_state = RTCM3_ST_DATA;
		break;

	case RTCM3_ST_DATA:
		if (p->irq_buf_pos < RTCM3_BUF_SIZE) {
			p->bufs[p->in_buf_idx][p->irq_buf_pos++] = byte;
		}
		if (--p->irq_bytes_remaining == 0) {
			p->buf_len[p->in_buf_idx] = p->irq_buf_pos;
			p->ready_mask |= (1u << p->in_buf_idx);
			p->in_buf_idx = (p->in_buf_idx + 1) % RTCM3_BUF_COUNT;
			p->irq_state = RTCM3_ST_SEARCH;
			p->irq_buf_pos = 0;
		}
		break;
	}
}

void rtcm3_loop(rtcm3_t *p)
{
	if(!p->ready_mask) {
		return;
	}

	int      idx         = p->out_buf_idx;
	uint8_t *frame       = p->bufs[idx];
	uint16_t payload_len = ((uint16_t)(frame[1] & 0x03) << 8) | frame[2];
	uint16_t frame_len   = p->buf_len[idx];

	uint32_t calc_crc = crc24q(frame, 3 + payload_len);
	uint32_t recv_crc = ((uint32_t)frame[3 + payload_len]     << 16)
	                  | ((uint32_t)frame[3 + payload_len + 1] <<  8)
	                  |  (uint32_t)frame[3 + payload_len + 2];

	if(calc_crc == recv_crc && p->on_frame) {
		p->on_frame(frame, frame_len);
	}

	p->ready_mask  &= ~(1u << idx);
	p->out_buf_idx  = (p->out_buf_idx + 1) % RTCM3_BUF_COUNT;
}
