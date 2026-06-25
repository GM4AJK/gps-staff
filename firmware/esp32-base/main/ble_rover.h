#pragma once
#include "driver/uart.h"

void ble_rover_init(uart_port_t uart);
void ble_rover_on_sync(void);
