#include "task_ota.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "main.h"
#include "rtcm3.h"
#include "ota_tx.h"
#include "sx1262.h"

static rtcm3_t  rtcm3;
static ota_tx_t ota_tx;
static sx1262_t sx1262;

static SemaphoreHandle_t dio1_sem;

void HAL_GPIO_EXTI_Callback(uint16_t gpio_pin)
{
	if (gpio_pin == SX1262_SPI_DIO1_Pin) {
		BaseType_t woken = pdFALSE;
		xSemaphoreGiveFromISR(dio1_sem, &woken);
		portYIELD_FROM_ISR(woken);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2) {
		/* Re-arm IT before calling rtcm3_uart_in_irq. The re-arm puts the
		 * UART back into BUSY_RX so the HAL_UART_Receive() call inside
		 * rtcm3_uart_in_irq hits the busy check and returns immediately
		 * instead of blocking — which would deadlock in FreeRTOS because
		 * the HAL tick (TIM14) can't preempt this ISR. */
		HAL_UART_Receive_IT(huart, &rtcm3.irq_rx_byte, 1);
		rtcm3_uart_in_irq(&rtcm3);
	}
}

static void on_rtcm3_frame(const uint8_t *frame, uint16_t len)
{
	ota_tx_push_frame(&ota_tx, frame, len);
}

static void ota_task(void *arg)
{
	(void)arg;

	sx1262_init(&sx1262, &hspi2,
		SX1262_SPI_CS_GPIO_Port,    SX1262_SPI_CS_Pin,
		SX1262_SPI_RESET_GPIO_Port, SX1262_SPI_RESET_Pin,
		SX1262_SPI_BUSY_GPIO_Port,  SX1262_SPI_BUSY_Pin);

	sx1262_config_gfsk(&sx1262, 434000000UL, 50000, 25000, OTA_TX_PACKET_SIZE, 0);

	ota_tx_init(&ota_tx, &sx1262);
	rtcm3_init(&rtcm3, &huart2, on_rtcm3_frame);

	for (;;) {
		rtcm3_loop(&rtcm3);
		if (xSemaphoreTake(dio1_sem, pdMS_TO_TICKS(1)) == pdTRUE)
			sx1262_service_tx(&sx1262);
	}
}

void task_ota_init(void)
{
	dio1_sem = xSemaphoreCreateBinary();
	configASSERT(dio1_sem);
	xTaskCreate(ota_task, "ota", 512, NULL, tskIDLE_PRIORITY + 2, NULL);
}
