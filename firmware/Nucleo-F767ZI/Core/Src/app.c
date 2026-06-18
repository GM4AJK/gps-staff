

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define TEST_SDCARD

#include "main.h"
#include "fatfs.h"
#include "bsp_driver_sd.h"
#include "flags.h"
#include "rtcm3.h"
#include "ota_tx.h"
#include "ssd1309.h"
#include "Tests/test_ssd1309.h"
#include "sx1262.h"

static void app_tests(void);

#ifdef TEST_SDCARD
/* Diagnostic: override weak BSP_SD_Init() to force 1-bit mode.
 * DATA_CRC_FAIL in 4-bit mode suggests D1/D2/D3 wiring issue on the SD BOB.
 * Remove once 4-bit wiring is confirmed or fixed. */
uint8_t BSP_SD_Init(void)
{
    extern SD_HandleTypeDef hsd2;
    if (BSP_SD_IsDetected() != SD_PRESENT)
        return MSD_ERROR_SD_NOT_PRESENT;
    return (HAL_SD_Init(&hsd2) == HAL_OK) ? MSD_OK : MSD_ERROR;
}
#endif

rtcm3_t rtcm3;
static ota_tx_t ota_tx;
static ssd1309_t oled;
static sx1262_t sx1262;

void HAL_GPIO_EXTI_Callback(uint16_t gpio_pin)
{
	if (gpio_pin == SX1262_SPI_DIO1_Pin) {
		flag_set_SX1262_DIO1();
	}
}

#define COUNTER_TIMER(x, y, z) \
	static volatile int x = 0; \
	x++; \
	if(x >= y) { \
		x = 0; \
		z(); \
	}

void app_1ms(void)
{
	COUNTER_TIMER(   cnt_10ms,   10, flag_set_10MS   );
	COUNTER_TIMER(  cnt_100ms,  100, flag_set_100MS  );
	COUNTER_TIMER( cnt_1000ms, 1000, flag_set_1000MS );
}

void app_log(const char *fmt, ...)
{
	char buf[128];
	va_list args;

	va_start(args, fmt);
	int len = vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 100);
}

#ifdef SX1262_WITH_LOGGING
static void sx1262_logger(const char *buf, int len)
{
	HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 100);
}
#endif /* SX1262_WITH_LOGGING */

static void on_rtcm3_frame(const uint8_t *frame, uint16_t len)
{
	ota_tx_push_frame(&ota_tx, frame, len);
}

void app_init(void)
{
	/* Enable DWT cycle counter for timing diagnostics.
	 * Cortex-M7's DWT block is behind a lock register - writes to CTRL/CYCCNT
	 * are silently ignored until it's unlocked with this magic value. */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->LAR = 0xC5ACCE55;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	/* Allow externally connected devices time to power up before init */
	HAL_Delay(500);

	app_log("F767ZI starting\r\n");

	ssd1309_init(&oled, &hi2c1, 0x3C, -1, -1);

	if (ssd1309_bringup(&oled) != HAL_OK) {
		app_log("ssd1309_bringup failed\r\n");
		return;
	}

	sx1262_init(
		&sx1262, &hspi2,
		SX1262_SPI_CS_GPIO_Port, SX1262_SPI_CS_Pin,
		SX1262_SPI_RESET_GPIO_Port, SX1262_SPI_RESET_Pin,
		SX1262_SPI_BUSY_GPIO_Port, SX1262_SPI_BUSY_Pin
	);

#ifdef SX1262_WITH_LOGGING
	sx1262_set_logger_callback(&sx1262, sx1262_logger);
#endif /* SX1262_WITH_LOGGING */

	if(sx1262_config_gfsk(&sx1262, 434000000UL, 50000, 25000, OTA_TX_PACKET_SIZE, 0) != HAL_OK) {
		app_log("sx1262: config gfsk failed\r\n");
		return;
	}

	rtcm3_init(&rtcm3, &huart2, on_rtcm3_frame);
	ota_tx_init(&ota_tx, &sx1262);

	app_log("F767ZI Base Start up\r\n");

	app_tests();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2) {
		rtcm3_uart_in_irq(&rtcm3);
	}
}

void app_loop(void)
{
	bool flipper = false;

	while(true) {
		rtcm3_loop(&rtcm3);

		if(flag_get_100MS()) {
			HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, flipper);
			flipper = !flipper;
		}

		if(flag_get_SX1262_DIO1()) {
			sx1262_service_tx(&sx1262);
		}
	}
}

static void app_tests(void)
{
#ifdef TEST_SSD1309
	HAL_StatusTypeDef r = test_ssd1309_shapes(&oled);
	if (r != HAL_OK) {
		app_log("ssd1309_flush failed: %d\r\n", r);
	}
#endif /* TEST_SSD1309 */

#ifdef TEST_SDCARD
	extern SD_HandleTypeDef hsd2;
	static const char *test_path = "test.txt";
	static const char *test_str  = "gps-staff sdcard ok\r\n";
	FRESULT fr;
	UINT bw, br;
	char rbuf[32];

	fr = f_mount(&SDFatFS, SDPath, 1);
	app_log("sd: mount %d err=0x%08lx state=%d\r\n", fr, hsd2.ErrorCode, (int)hsd2.State);
	if (fr != FR_OK) goto sd_done;

	fr = f_open(&SDFile, test_path, FA_CREATE_ALWAYS | FA_WRITE);
	app_log("sd: open(w) %d\r\n", fr);
	if (fr != FR_OK) goto sd_unmount;

	fr = f_write(&SDFile, test_str, strlen(test_str), &bw);
	app_log("sd: write %d bytes, fr=%d\r\n", bw, fr);
	f_close(&SDFile);

	fr = f_open(&SDFile, test_path, FA_READ);
	app_log("sd: open(r) %d\r\n", fr);
	if (fr != FR_OK) goto sd_unmount;

	fr = f_read(&SDFile, rbuf, sizeof(rbuf) - 1, &br);
	rbuf[br] = '\0';
	app_log("sd: read '%s' fr=%d\r\n", rbuf, fr);
	f_close(&SDFile);

sd_unmount:
	f_mount(NULL, SDPath, 0);
sd_done:;
#endif /* TEST_SDCARD */
}
