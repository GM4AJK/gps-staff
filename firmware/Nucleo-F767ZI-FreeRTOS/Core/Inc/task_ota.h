#pragma once

/**
 * task_ota_init
 *
 * Creates the DIO1 binary semaphore and spawns the OTA task, which owns
 * the rtcm3, ota_tx, and sx1262 instances. Must be called once while the
 * scheduler is running, before USART2 interrupt traffic begins.
 */
void task_ota_init(void);
