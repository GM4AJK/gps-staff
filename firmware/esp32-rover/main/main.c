#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define TAG "rover"

void app_main(void)
{
	ESP_LOGI(TAG, "ESP32-S3 Rover — BLE central + UART TX to Fake-F9P");

	while (1) {
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
