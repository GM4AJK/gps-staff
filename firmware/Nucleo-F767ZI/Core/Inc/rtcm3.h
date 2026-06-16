/*
 * rtcm3.h -- RTCM3 framing module
 *
 * Receives a raw RTCM3 byte stream from a UART (e.g. a ZED-F9P or a
 * Fake-F9P board), frames complete messages into a buffer pool, validates
 * the CRC24Q checksum, and signals the idle loop that a frame is ready.
 *
 * ---------------------------------------------------------------------------
 * BUFFER POOL
 * ---------------------------------------------------------------------------
 * RTCM3_BUF_COUNT (4) buffers of RTCM3_BUF_SIZE (2048) bytes each hold
 * complete raw frames including the 0xD3 preamble, 2-byte length, payload,
 * and 3-byte CRC. Incoming frames are dropped silently when all slots are
 * full (i.e. rtcm3_loop() is not consuming fast enough).
 *
 * ---------------------------------------------------------------------------
 * INTEGRATION STEPS
 * ---------------------------------------------------------------------------
 *
 * 1. CALLER OWNS THE INSTANCE -- define it at file scope (NOT static) in
 *    the file that also defines HAL_UART_RxCpltCallback, typically app.c:
 *
 *        rtcm3_t rtcm3;
 *
 *    It must be non-static so that stm32f7xx_it.c can reference it (see
 *    step 3). If you need to reference it from other translation units,
 *    add an extern declaration in your app.h:
 *
 *        extern rtcm3_t rtcm3;
 *
 * 2. INITIALISE -- call once from app_init(), after all MX_*_Init() calls
 *    have run and the target UART is ready:
 *
 *        rtcm3_init(&rtcm3, &huart2, &huart3);  // f9p UART, debug UART
 *
 *    rtcm3_init() arms HAL_UART_Receive_IT() for the first byte. The UART's
 *    NVIC interrupt must already be enabled (do this via CubeMX so that
 *    HAL_UART_MspInit() configures the priority and enables the IRQ).
 *
 * 3. WIRE THE IRQ -- in stm32f7xx_it.c, add an extern declaration and call
 *    rtcm3_uart_in_irq() at the TOP of the peripheral's IRQ handler, before
 *    HAL_UART_IRQHandler(), so the byte is captured while RXNE is still set:
 *
 *        // In the USER CODE EV section:
 *        extern rtcm3_t rtcm3;
 *
 *        // The IRQ handler (CubeMX generates the shell, add the USER CODE):
 *        void USART2_IRQHandler(void)
 *        {
 *            // USER CODE BEGIN USART2_IRQn 0
 *            rtcm3_uart_in_irq(&rtcm3);
 *            // USER CODE END USART2_IRQn 0
 *            HAL_UART_IRQHandler(&huart2);
 *        }
 *
 * 4. SERVICE THE LOOP -- call from app_loop() on every iteration:
 *
 *        rtcm3_loop(&rtcm3);
 *
 *    rtcm3_loop() checks whether a complete frame is waiting, validates its
 *    CRC24Q, logs the result over the debug UART, then releases the buffer.
 *
 * ---------------------------------------------------------------------------
 * STARTUP NOTE
 * ---------------------------------------------------------------------------
 * The very first frame after power-on will log "CRC FAIL". This is a known
 * one-byte lag: on the first IRQ firing irq_rx_byte still holds its initial
 * zero value, so the state machine processes a corrupted first byte before
 * the real stream aligns. Every subsequent frame is unaffected.
 */

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
