#pragma once
#include "driver/uart.h"

void ble_base_register_svcs(void);
void ble_base_on_sync(void);
void ble_base_start_tasks(uart_port_t uart);
