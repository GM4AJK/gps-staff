

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "flags.h"
#include "ssd1309.h"
#include "bno085.h"

static ssd1309_t oled;
static bno085_t bno;

#define COUNTER_TIMER(x, y, z) \
	x++; \
	if(x >= y) { \
		x = 0; \
		z(); \
	}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == BNO085_INT_Pin) {
		flag_set_BNO085_INT();
	}
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
	/* Allow externally connected devices time to power up before init */
	HAL_Delay(500);

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

	bno085_init(&bno, &hspi1,
		BNO085_SPI_CS_GPIO_Port, BNO085_SPI_CS_Pin,
		BNO085_RST_GPIO_Port, BNO085_RST_Pin,
		BNO085_INT_GPIO_Port, BNO085_INT_Pin,
		BNO085_PS0_WAKE_GPIO_Port, BNO085_PS0_WAKE_Pin);

	HAL_StatusTypeDef bno_status = bno085_bringup(&bno);
	char buf[96];
	int len;

	len = snprintf(buf, sizeof(buf), "bno085 INT initial=%d wait=%lums\r\n",
		bno.int_initial, (unsigned long)bno.int_wait_ms);
	HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 100);

	if (bno_status == HAL_OK) {
		len = snprintf(buf, sizeof(buf), "bno085_bringup OK: len=%u chan=%u seq=%u\r\n",
			bno.shtp_length, bno.shtp_channel, bno.shtp_sequence);
	} else {
		len = snprintf(buf, sizeof(buf), "bno085_bringup failed: %d\r\n", bno_status);
	}
	HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 100);

	len = snprintf(buf, sizeof(buf), "bno085 rx_buf:");
	HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 100);
	for (int i = 0; i < BNO085_BRINGUP_BUF_SIZE; i++) {
		len = snprintf(buf, sizeof(buf), " %02X", bno.rx_buf[i]);
		HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 100);
	}
	HAL_UART_Transmit(&huart3, (uint8_t *)"\r\n", 2, 100);

	HAL_StatusTypeDef advert_status = bno085_read_advertisement(&bno);

	if (advert_status == HAL_OK) {
		len = snprintf(buf, sizeof(buf), "bno085_read_advertisement OK: advert_len=%u\r\n", bno.advert_len);
		HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 100);
		bno085_print_advertisement(&bno, &huart3);
	} else {
		len = snprintf(buf, sizeof(buf), "bno085_read_advertisement failed: %d\r\n", advert_status);
		HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 100);
	}

	static const struct {
		uint8_t report_id;
		const char *name;
	} sensors[] = {
		{ 0x01, "Accelerometer" },
		{ 0x02, "Gyroscope" },
		{ 0x03, "Magnetic Field" },
		{ 0x05, "Rotation Vector" },
		{ 0x08, "Game Rotation Vector" },
	};

	for (size_t i = 0; i < sizeof(sensors) / sizeof(sensors[0]); i++) {
		HAL_StatusTypeDef feature_status = bno085_get_feature(&bno, sensors[i].report_id);

		if (feature_status == HAL_OK) {
			len = snprintf(buf, sizeof(buf), "%s: interval=%luus batch=%luus flags=%02X sens=%u\r\n",
				sensors[i].name,
				(unsigned long)bno.feature.report_interval_us,
				(unsigned long)bno.feature.batch_interval_us,
				bno.feature.feature_flags,
				bno.feature.change_sensitivity);
		} else {
			len = snprintf(buf, sizeof(buf), "%s: bno085_get_feature failed: %d (int_wait=%lums)\r\n",
				sensors[i].name, feature_status, (unsigned long)bno.int_wait_ms);
		}
		HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 100);

		len = snprintf(buf, sizeof(buf), "  cmd_len=%u cmd_buf:", bno.cmd_len);
		HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 100);
		for (int j = 0; j < BNO085_CMD_BUF_SIZE; j++) {
			len = snprintf(buf, sizeof(buf), " %02X", bno.cmd_buf[j]);
			HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 100);
		}
		HAL_UART_Transmit(&huart3, (uint8_t *)"\r\n", 2, 100);
	}

	/* Enable the Rotation Vector report at 20Hz (50000us interval), no
	 * batching, default change sensitivity/flags/sensor-specific config. */
	HAL_StatusTypeDef set_feature_status = bno085_set_feature(&bno, 0x05, 0, 0, 50000, 0, 0);

	if (set_feature_status == HAL_OK) {
		len = snprintf(buf, sizeof(buf), "bno085_set_feature(Rotation Vector) OK\r\n");
	} else {
		len = snprintf(buf, sizeof(buf), "bno085_set_feature(Rotation Vector) failed: %d\r\n", set_feature_status);
	}
	HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 100);

	/* Enable the Magnetic Field Calibrated report at the same 20Hz rate, as
	 * a diagnostic alongside the Rotation Vector's status/accuracy. */
	HAL_StatusTypeDef set_feature_mag_status = bno085_set_feature(&bno, 0x03, 0, 0, 50000, 0, 0);

	if (set_feature_mag_status == HAL_OK) {
		len = snprintf(buf, sizeof(buf), "bno085_set_feature(Magnetic Field) OK\r\n");
	} else {
		len = snprintf(buf, sizeof(buf), "bno085_set_feature(Magnetic Field) failed: %d\r\n", set_feature_mag_status);
	}
	HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 100);

	/* Enable accel/gyro/mag dynamic calibration (planar/on-table left
	 * disabled) so the Rotation Vector's status/accuracy can improve. */
	HAL_StatusTypeDef me_cal_status = bno085_set_me_calibration(&bno, 1, 1, 1, 0, 0);

	if (me_cal_status == HAL_OK) {
		len = snprintf(buf, sizeof(buf), "bno085_set_me_calibration OK: status=%u accel=%u gyro=%u mag=%u\r\n",
			bno.me_calibration.status, bno.me_calibration.accel_enable,
			bno.me_calibration.gyro_enable, bno.me_calibration.mag_enable);
	} else {
		len = snprintf(buf, sizeof(buf), "bno085_set_me_calibration failed: %d\r\n", me_cal_status);
	}
	HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 100);
}

void app_loop(void)
{
	bool flipper = false;
	uint32_t loop_count = 0;
	uint32_t me_cal_count = 0;
	char buf[64];

	while(true) {
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, flipper);
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, !flipper);
		flipper = !flipper;

		//int len = snprintf(buf, sizeof(buf), "loop %lu\r\n", loop_count++);
		//HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 100);

		//if (flag_get_100MS()) {
			if (bno085_read_rotation_vector(&bno) == HAL_OK) {
				//bno085_print_rotation_vector(&bno, &huart3);
			}

			if (bno085_read_magnetic_field(&bno) == HAL_OK) {
				bno085_print_magnetic_field(&bno, &huart3);
			}
		//}

		/* Every ~2s, read back the on-chip ME calibration enable state to
		 * confirm the Configure command persisted. */
		if (++me_cal_count >= 4) {
			me_cal_count = 0;

			int len;
			if (bno085_get_me_calibration(&bno) == HAL_OK) {
				len = snprintf(buf, sizeof(buf), "me_cal: status=%u accel=%u gyro=%u mag=%u\r\n",
					bno.me_calibration.status, bno.me_calibration.accel_enable,
					bno.me_calibration.gyro_enable, bno.me_calibration.mag_enable);
			} else {
				len = snprintf(buf, sizeof(buf), "bno085_get_me_calibration failed\r\n");
			}
			HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 100);
		}

		HAL_Delay(500);
	}
}
