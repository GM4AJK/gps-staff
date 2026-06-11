

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "flags.h"
#include "ssd1309.h"
//#include "bno085.h"
#include "bno085_spi.h"

static ssd1309_t oled;
//static bno085_t bno;

void bno085_init(void);
void bno085_service(void);

#define COUNTER_TIMER(x, y, z) \
	x++; \
	if(x >= y) { \
		x = 0; \
		z(); \
	}

void app_delay_us(uint32_t us)
{
	uint32_t target = DWT->CYCCNT + us * (SystemCoreClock / 1000000U);
	while((int32_t)(DWT->CYCCNT - target) < 0);
}

#define DWT_LAR_UNLOCK_KEY  0xC5ACCE55

static void app_delay_us_init()
{
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL        |= DWT_CTRL_CYCCNTENA_Msk;
	if ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) == 0) {
		// Enable use of DWT and ITM blocks
		CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	}
	// 2. Unlock DWT access (Mandatory for Cortex-M7)
	// Without this, writes to DWT control registers are ignored
	DWT->LAR = DWT_LAR_UNLOCK_KEY;
	// 3. Reset the cycle counter register
	DWT->CYCCNT = 0;
	// 4. Enable the cycle counter
	// Bit 0 (CYCCNTENA) enables the counter
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	// 5. Verification check
//	if (DWT->CYCCNT == DWT->CYCCNT) {
//		return 0; // Success, counter is running
//	}

}

void app_1ms(void)
{
	static volatile int cnt_10ms = 0;
	static volatile int cnt_100ms = 0;
	static volatile int cnt_1000ms = 0;
	COUNTER_TIMER(   cnt_10ms,   10, flag_set_10MS   );
	COUNTER_TIMER(  cnt_100ms,  100, flag_set_100MS  );
	COUNTER_TIMER( cnt_1000ms, 1000, flag_set_1000MS );
}

void app_init(void)
{
	app_delay_us_init();

	/* Allow externally connected devices time to power up before init */
	HAL_Delay(500);

//	bno.spi = &hspi1;
//	bno.reset.pin        = BNO085_RST_Pin;
//	bno.reset.port       = BNO085_RST_GPIO_Port;
//	bno.ps0_wake.pin     = BNO085_PS0_WAKE_Pin;
//	bno.ps0_wake.port    = BNO085_PS0_WAKE_GPIO_Port;
//	bno.interrupt.pin    = BNO085_INT_Pin;
//	bno.interrupt.port   = BNO085_INT_GPIO_Port;
//	bno.chip_select.pin  = BNO085_SPI_CS_Pin;
//	bno.chip_select.port = BNO085_SPI_CS_GPIO_Port;
//	bno.us_delay_fp      = app_delay_us;
//	bno085_init(&bno);

	bno085_init();

	ssd1309_init(&oled, &hi2c1, 0x3C, -1, -1);

	if (ssd1309_bringup(&oled) != HAL_OK) {
		const char *msg = "ssd1309_bringup failed\r\n";
		HAL_UART_Transmit(&huart3, (uint8_t *)msg, strlen(msg), 100);
		return;
	}

	ssd1309_clear(&oled);
	ssd1309_draw_string(&oled, &font5x7, 0, 0, "Nucleo-F767ZI OK", SSD1309_COLOR_ON);
	ssd1309_draw_line(&oled, 0, 9, oled.width - 1, 9, SSD1309_COLOR_ON);
	ssd1309_draw_rect(&oled, 0, 14, 20, 34, false, SSD1309_COLOR_ON);
	ssd1309_draw_rect(&oled, 26, 14, 46, 34, true, SSD1309_COLOR_ON);
	ssd1309_draw_circle(&oled, 75, 24, 10, false, SSD1309_COLOR_ON);
	ssd1309_draw_circle(&oled, 105, 24, 10, true, SSD1309_COLOR_ON);
	ssd1309_draw_arrow(&oled, 10, 45, 50, 60, 6, SSD1309_COLOR_ON);
	ssd1309_draw_triangle(&oled, 70, 45, 90, 45, 80, 62, false, SSD1309_COLOR_ON);

	static const ssd1309_point_t pentagon[] = {
		{ 59, 45 },
		{ 67, 51 },
		{ 64, 60 },
		{ 54, 60 },
		{ 51, 51 },
	};
	ssd1309_draw_polygon(&oled, pentagon, 5, true, SSD1309_COLOR_ON);

	static const ssd1309_point_t hexagon[] = {
		{ 105, 46 },
		{ 112, 50 },
		{ 112, 58 },
		{ 105, 62 },
		{ 98, 58 },
		{ 98, 50 },
	};
	ssd1309_draw_polygon(&oled, hexagon, 6, false, SSD1309_COLOR_ON);

	if (ssd1309_flush(&oled) != HAL_OK) {
		const char *msg = "ssd1309_flush failed\r\n";
		HAL_UART_Transmit(&huart3, (uint8_t *)msg, strlen(msg), 100);
	}


}

void app_loop(void)
{
	bool flipper = false;

	while(true) {
		bno085_service();
//		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, flipper);
//		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, !flipper);
//		flipper = !flipper;
//		HAL_Delay(500);
	}
}
