#include "Tests/test_imu_fusion.h"
#include "app.h"
#include <stdio.h>

#ifdef TEST_IMU_FUSION

static ssd1309_t *oled = NULL;

void test_imu_fusion_set_oled(ssd1309_t *p)
{
	oled = p;
}

/* Splits a degrees value into sign/integer/centi-degree parts for
 * printing without relying on float support in printf/snprintf. */
static void split_centideg(float deg, const char **out_sign, int *out_int, int *out_frac)
{
	int centideg = (int)(deg * 100.0f);

	if (centideg < 0) {
		centideg = -centideg;
		*out_sign = "-";
	} else {
		*out_sign = "";
	}

	*out_int = centideg / 100;
	*out_frac = centideg % 100;
}

void test_imu_fusion_poll(lsm6dsrx_t *imu, lis3mdl_t *mag)
{
	int16_t ax, ay, az, mx, my, mz;
	float roll_deg, pitch_deg, heading_deg;
	const char *roll_sign, *pitch_sign, *heading_sign;
	int roll_int, roll_frac, pitch_int, pitch_frac, heading_int, heading_frac;
	char line[24];

	if (lsm6dsrx_read_accel(imu, &ax, &ay, &az) != HAL_OK) {
		return;
	}
	lsm6dsrx_compute_tilt(ax, ay, az, &roll_deg, &pitch_deg);

	if (lis3mdl_read_mag(mag, &mx, &my, &mz) != HAL_OK) {
		return;
	}
	lis3mdl_compute_heading(mx, my, mz, roll_deg, pitch_deg, &heading_deg);

	split_centideg(roll_deg, &roll_sign, &roll_int, &roll_frac);
	split_centideg(pitch_deg, &pitch_sign, &pitch_int, &pitch_frac);
	split_centideg(heading_deg, &heading_sign, &heading_int, &heading_frac);

	app_log(
		"imu_fusion: roll=%s%d.%02d pitch=%s%d.%02d heading=%s%d.%02d\r\n",
		roll_sign, roll_int, roll_frac,
		pitch_sign, pitch_int, pitch_frac,
		heading_sign, heading_int, heading_frac
	);

	if (oled == NULL) {
		return;
	}

	ssd1309_clear(oled);

	snprintf(line, sizeof(line), "Roll:  %s%d.%02d", roll_sign, roll_int, roll_frac);
	ssd1309_draw_string(oled, &font5x7, 0, 0, line, SSD1309_COLOR_ON);

	snprintf(line, sizeof(line), "Pitch: %s%d.%02d", pitch_sign, pitch_int, pitch_frac);
	ssd1309_draw_string(oled, &font5x7, 0, 10, line, SSD1309_COLOR_ON);

	snprintf(line, sizeof(line), "Hdg:   %s%d.%02d", heading_sign, heading_int, heading_frac);
	ssd1309_draw_string(oled, &font5x7, 0, 20, line, SSD1309_COLOR_ON);

	ssd1309_flush(oled);
}

#endif /* TEST_IMU_FUSION */
