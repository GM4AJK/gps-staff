#pragma once

/**
 * task_ota_init
 *
 * Creates the DIO1 binary semaphore and spawns the OTA task, which owns
 * the rtcm3, ota_tx, and sx1262 instances. Must be called once while the
 * scheduler is running, before USART2 interrupt traffic begins.
 */
void task_ota_init(void);

/**
 * task_ota_uart2_irq
 *
 * Call at the top of USART2_IRQHandler (USER CODE BEGIN 0), before
 * HAL_UART_IRQHandler(). Forwards the received byte into the rtcm3
 * state machine.
 */
void task_ota_uart2_irq(void);
