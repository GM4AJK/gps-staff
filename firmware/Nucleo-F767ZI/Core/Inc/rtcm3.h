#ifndef INC_RTCM3_H_
#define INC_RTCM3_H_

#include <stdint.h>
#include "main.h"

#define RTCM3_BUF_COUNT  4
#define RTCM3_BUF_SIZE   2048

typedef enum {
	RTCM3_ST_SEARCH,
	RTCM3_ST_LEN1,
	RTCM3_ST_LEN2,
	RTCM3_ST_DATA,
} rtcm3_irq_state_t;

typedef struct {
	UART_HandleTypeDef        *uart_f9p;
	UART_HandleTypeDef        *uart_dbg;
	uint8_t                    bufs[RTCM3_BUF_COUNT][RTCM3_BUF_SIZE];
	uint16_t                   buf_len[RTCM3_BUF_COUNT];
	volatile int               in_buf_idx;
	int                        out_buf_idx;
	volatile uint8_t           ready_mask;
	uint8_t                    irq_rx_byte;
	volatile rtcm3_irq_state_t irq_state;
	volatile uint16_t          irq_bytes_remaining;
	volatile uint16_t          irq_buf_pos;
} rtcm3_t;

void rtcm3_init(rtcm3_t *p, UART_HandleTypeDef *f9p_uart, UART_HandleTypeDef *debug_uart);
void rtcm3_uart_in_irq(rtcm3_t *p);
void rtcm3_loop(rtcm3_t *p);

#endif /* INC_RTCM3_H_ */
