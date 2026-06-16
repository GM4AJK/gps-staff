#ifndef INC_RTCM3_H_
#define INC_RTCM3_H_

#include "main.h"

#define RTCM3_BUF_COUNT  4
#define RTCM3_BUF_SIZE   2048

void rtcm3_init(UART_HandleTypeDef *f9p_uart, UART_HandleTypeDef *debug_uart);
void rtcm3_uart_in_irq(void);
void rtcm3_loop(void);

#endif /* INC_RTCM3_H_ */
