/*
 * rtcm3.h -- RTCM3 framing module
 *
 * Receives a raw RTCM3 byte stream, frames complete messages into a buffer
 * pool, validates CRC24Q, and fires a caller-supplied callback for each valid
 * frame.
 *
 * ---------------------------------------------------------------------------
 * FREERTOS USAGE (preferred)
 * ---------------------------------------------------------------------------
 * A UART ISR reads RDR directly and pushes bytes to a FreeRTOS queue.  The
 * OTA task drains the queue and calls rtcm3_process_byte() per byte.  After
 * draining, it calls rtcm3_loop() to dispatch any completed frames.  No HAL
 * calls in the interrupt path.
 *
 * ---------------------------------------------------------------------------
 * BARE-METAL USAGE (reference)
 * ---------------------------------------------------------------------------
 * Call rtcm3_uart_in_irq() from HAL_UART_RxCpltCallback(); it reads
 * irq_rx_byte and calls rtcm3_process_byte() internally.  Call rtcm3_loop()
 * from the idle loop to dispatch completed frames.
 *
 * ---------------------------------------------------------------------------
 * BUFFER POOL
 * ---------------------------------------------------------------------------
 * RTCM3_BUF_COUNT (4) buffers of RTCM3_BUF_SIZE (2048) bytes hold complete
 * raw frames including preamble, 2-byte length, payload, and 3-byte CRC.
 * Incoming frames are silently dropped when all slots are full.
 */

#ifndef INC_RTCM3_H_
#define INC_RTCM3_H_

#include <stdint.h>

#define RTCM3_BUF_COUNT  4
#define RTCM3_BUF_SIZE   2048

typedef enum {
	RTCM3_ST_SEARCH,
	RTCM3_ST_LEN1,
	RTCM3_ST_LEN2,
	RTCM3_ST_DATA,
} rtcm3_irq_state_t;

typedef struct {
	void             (*on_frame)(const uint8_t *frame, uint16_t len);
	uint8_t            bufs[RTCM3_BUF_COUNT][RTCM3_BUF_SIZE];
	uint16_t           buf_len[RTCM3_BUF_COUNT];
	int                in_buf_idx;
	int                out_buf_idx;
	uint8_t            ready_mask;
	uint8_t            irq_rx_byte;        /* bare-metal path: written by HAL callback */
	rtcm3_irq_state_t  irq_state;
	uint16_t           irq_bytes_remaining;
	uint16_t           irq_buf_pos;
} rtcm3_t;

void rtcm3_init(rtcm3_t *p, void (*on_frame)(const uint8_t *frame, uint16_t len));
void rtcm3_process_byte(rtcm3_t *p, uint8_t byte);
void rtcm3_uart_in_irq(rtcm3_t *p);    /* bare-metal path; calls rtcm3_process_byte */
void rtcm3_loop(rtcm3_t *p);

#endif /* INC_RTCM3_H_ */
