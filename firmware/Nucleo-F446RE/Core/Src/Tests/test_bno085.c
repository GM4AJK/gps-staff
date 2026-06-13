
#include "Tests/test_bno085.h"
#include "app.h"
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#ifdef TEST_BNO085

#define BNO085_PRODUCT_ID_ENTRY_SIZE 16

static void hex_dump(char *hex, size_t hex_size, const uint8_t *buf, uint16_t len)
{
	size_t pos = 0;
	for (uint16_t i = 0; i < len; i++) {
		pos += (size_t)snprintf(&hex[pos], hex_size - pos, "%02X ", buf[i]);
	}
	hex[pos] = '\0';
}

static const char *channel_name(uint8_t channel)
{
	switch (channel) {
	case BNO085_CHANNEL_COMMAND: return "command";
	case 1: return "executable";
	case BNO085_CHANNEL_CONTROL: return "control";
	case 3: return "reports";
	case 4: return "wake reports";
	case 5: return "gyro rv";
	default: return "?";
	}
}

/* Decodes one 16-byte Product ID Response entry (SH-2 ref manual 6.4.5) */
static void print_product_id_entry(const uint8_t *e)
{
	uint32_t part = (uint32_t)e[4] | ((uint32_t)e[5] << 8) | ((uint32_t)e[6] << 16) | ((uint32_t)e[7] << 24);
	uint32_t build = (uint32_t)e[8] | ((uint32_t)e[9] << 8) | ((uint32_t)e[10] << 16) | ((uint32_t)e[11] << 24);
	uint16_t patch = (uint16_t)(e[12] | (e[13] << 8));

	app_log("bno085: product ID: SW v%u.%u.%u, part %lu, build %lu\r\n",
			e[2], e[3], patch, (unsigned long)part, (unsigned long)build);
}

/* Formats a Q-point fixed-point value as a decimal string with 4 fractional
 * digits, without relying on printf float support (--specs=nano.specs does
 * not include it by default) */
static void format_q(char *out, size_t out_size, int16_t raw, int shift)
{
	float val = (float)raw / (float)(1 << shift);
	int32_t scaled = (int32_t)(val * 10000.0f);
	bool negative = scaled < 0;

	if (negative) {
		scaled = -scaled;
	}

	snprintf(out, out_size, "%s%ld.%04ld", negative ? "-" : "", (long)(scaled / 10000), (long)(scaled % 10000));
}

/* Formats a float as a decimal string with 1 fractional digit, without
 * relying on printf float support (--specs=nano.specs does not include it
 * by default). Pads the whole part to 3 digits for column alignment. */
static void format_degrees(char *out, size_t out_size, float val)
{
	int32_t scaled = (int32_t)(val * 10.0f);
	bool negative = scaled < 0;

	if (negative) {
		scaled = -scaled;
	}

	snprintf(out, out_size, "%s%3ld.%01ld", negative ? "-" : "", (long)(scaled / 10), (long)(scaled % 10));
}

/* 8-point compass label for a 0-360 degree bearing */
static const char *compass_point(float bearing)
{
	static const char *points[8] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };
	int idx = (int)((bearing + 22.5f) / 45.0f) % 8;

	if (idx < 0) {
		idx += 8;
	}

	return points[idx];
}

/* Decodes a 14-byte Rotation Vector report (SH-2 ref manual 6.5.7): report
 * ID, sequence, status, delay, then i/j/k/real (Q14) and accuracy (Q12) */
static void print_rotation_vector(const uint8_t *r)
{
	int16_t i_raw = (int16_t)(r[4] | (r[5] << 8));
	int16_t j_raw = (int16_t)(r[6] | (r[7] << 8));
	int16_t k_raw = (int16_t)(r[8] | (r[9] << 8));
	int16_t real_raw = (int16_t)(r[10] | (r[11] << 8));
	int16_t acc_raw = (int16_t)(r[12] | (r[13] << 8));

	char i_s[16], j_s[16], k_s[16], real_s[16], acc_s[16];
	format_q(i_s, sizeof(i_s), i_raw, 14);
	format_q(j_s, sizeof(j_s), j_raw, 14);
	format_q(k_s, sizeof(k_s), k_raw, 14);
	format_q(real_s, sizeof(real_s), real_raw, 14);
	format_q(acc_s, sizeof(acc_s), acc_raw, 12);

	app_log("bno085: rotation vector: status=%u i=%s j=%s k=%s real=%s acc=%s\r\n",
			r[2] & 0x03u, i_s, j_s, k_s, real_s, acc_s);
}

void test_bno085_hello(bno085_t *p)
{
	if (bno085_probe(p) != HAL_OK) {
		app_log("bno085: probe failed\r\n");
		return;
	}
	app_log("bno085: probe OK\r\n");

	uint8_t buf[32];
	uint16_t len = 0;
	HAL_StatusTypeDef status = bno085_read_packet(p, buf, sizeof(buf), &len);
	if (status != HAL_OK) {
		app_log("bno085: read_packet failed: %d\r\n", status);
		return;
	}

	char hex[128];
	uint16_t print_len = (len < sizeof(buf)) ? len : (uint16_t)sizeof(buf);
	hex_dump(hex, sizeof(hex), buf, print_len);

	app_log("bno085: ch%u (%s) seq=%u, %u of %u bytes: %s\r\n",
			buf[2], channel_name(buf[2]), buf[3], print_len, len, hex);
}

void test_bno085_product_id(bno085_t *p)
{
	bno085_drain(p);

	uint8_t request[2] = { BNO085_REPORT_PRODUCT_ID_REQUEST, 0x00 };

	if (bno085_write_packet(p, BNO085_CHANNEL_CONTROL, request, sizeof(request)) != HAL_OK) {
		app_log("bno085: product ID request failed\r\n");
		return;
	}

	HAL_Delay(10);

	uint8_t buf[32];
	uint16_t len = 0;
	HAL_StatusTypeDef status = bno085_read_packet(p, buf, sizeof(buf), &len);
	if (status != HAL_OK) {
		app_log("bno085: product ID response read failed: %d\r\n", status);
		return;
	}

	uint16_t print_len = (len < sizeof(buf)) ? len : (uint16_t)sizeof(buf);

	if (buf[2] != BNO085_CHANNEL_CONTROL) {
		char hex[128];
		hex_dump(hex, sizeof(hex), buf, print_len);
		app_log("bno085: unexpected response on ch%u (%s) seq=%u, %u of %u bytes: %s\r\n",
				buf[2], channel_name(buf[2]), buf[3], print_len, len, hex);
		return;
	}

	const uint8_t *payload = &buf[BNO085_SHTP_HEADER_SIZE];
	uint16_t available = (uint16_t)(print_len - BNO085_SHTP_HEADER_SIZE);
	bool found = false;

	for (uint16_t off = 0; (uint16_t)(off + BNO085_PRODUCT_ID_ENTRY_SIZE) <= available; off += BNO085_PRODUCT_ID_ENTRY_SIZE) {
		if (payload[off] == BNO085_REPORT_PRODUCT_ID_RESPONSE) {
			print_product_id_entry(&payload[off]);
			found = true;
		}
	}

	if (!found) {
		char hex[128];
		hex_dump(hex, sizeof(hex), buf, print_len);
		app_log("bno085: no product ID entries, %u of %u bytes: %s\r\n", print_len, len, hex);
	}
}

void test_bno085_rotation_vector(bno085_t *p)
{
	bno085_drain(p);

	/* 100ms report interval (10Hz) */
	if (bno085_set_feature(p, BNO085_REPORT_ROTATION_VECTOR, 100000) != HAL_OK) {
		app_log("bno085: rotation vector set feature failed\r\n");
		return;
	}

	HAL_Delay(150);

	uint8_t buf[32];
	uint16_t len = 0;
	HAL_StatusTypeDef status = bno085_read_packet(p, buf, sizeof(buf), &len);
	if (status != HAL_OK) {
		app_log("bno085: rotation vector read failed: %d\r\n", status);
		return;
	}

	uint16_t print_len = (len < sizeof(buf)) ? len : (uint16_t)sizeof(buf);

	if (buf[2] != BNO085_CHANNEL_REPORTS) {
		char hex[128];
		hex_dump(hex, sizeof(hex), buf, print_len);
		app_log("bno085: unexpected response on ch%u (%s) seq=%u, %u of %u bytes: %s\r\n",
				buf[2], channel_name(buf[2]), buf[3], print_len, len, hex);
		return;
	}

	const uint8_t *payload = &buf[BNO085_SHTP_HEADER_SIZE];
	uint16_t available = (uint16_t)(print_len - BNO085_SHTP_HEADER_SIZE);
	uint16_t off = 0;

	if (off < available && payload[off] == BNO085_REPORT_BASE_TIMESTAMP_REFERENCE) {
		off += BNO085_BASE_TIMESTAMP_REFERENCE_SIZE;
	}

	if ((uint16_t)(off + BNO085_ROTATION_VECTOR_SIZE) <= available && payload[off] == BNO085_REPORT_ROTATION_VECTOR) {
		print_rotation_vector(&payload[off]);
	} else {
		char hex[128];
		hex_dump(hex, sizeof(hex), buf, print_len);
		app_log("bno085: no rotation vector report, %u of %u bytes: %s\r\n", print_len, len, hex);
	}
}

void test_bno085_rotation_vector_enable(bno085_t *p)
{
	bno085_drain(p);

	/* 100ms report interval (10Hz) */
	if (bno085_set_feature(p, BNO085_REPORT_ROTATION_VECTOR, 100000) != HAL_OK) {
		app_log("bno085: rotation vector set feature failed\r\n");
	}
}

void test_bno085_rotation_vector_display(bno085_t *p, ssd1309_t *oled)
{
	uint8_t buf[32];
	uint16_t len = 0;

	if (bno085_read_packet(p, buf, sizeof(buf), &len) != HAL_OK) {
		return;
	}

	uint16_t print_len = (len < sizeof(buf)) ? len : (uint16_t)sizeof(buf);

	if (buf[2] != BNO085_CHANNEL_REPORTS) {
		return;
	}

	const uint8_t *payload = &buf[BNO085_SHTP_HEADER_SIZE];
	uint16_t available = (uint16_t)(print_len - BNO085_SHTP_HEADER_SIZE);
	uint16_t off = 0;

	if (off < available && payload[off] == BNO085_REPORT_BASE_TIMESTAMP_REFERENCE) {
		off += BNO085_BASE_TIMESTAMP_REFERENCE_SIZE;
	}

	if (!((uint16_t)(off + BNO085_ROTATION_VECTOR_SIZE) <= available && payload[off] == BNO085_REPORT_ROTATION_VECTOR)) {
		return;
	}

	const uint8_t *r = &payload[off];
	int16_t i_raw = (int16_t)(r[4] | (r[5] << 8));
	int16_t j_raw = (int16_t)(r[6] | (r[7] << 8));
	int16_t k_raw = (int16_t)(r[8] | (r[9] << 8));
	int16_t real_raw = (int16_t)(r[10] | (r[11] << 8));
	uint8_t accuracy = r[2] & 0x03u;

	/* Q14 */
	float qi = (float)i_raw / 16384.0f;
	float qj = (float)j_raw / 16384.0f;
	float qk = (float)k_raw / 16384.0f;
	float qr = (float)real_raw / 16384.0f;

	const float rad_to_deg = 57.29578f;
	float roll  = atan2f(2.0f * (qr * qi + qj * qk), 1.0f - 2.0f * (qi * qi + qj * qj)) * rad_to_deg;
	float pitch = asinf(fmaxf(-1.0f, fminf(1.0f, 2.0f * (qr * qj - qk * qi)))) * rad_to_deg;
	float yaw   = atan2f(2.0f * (qr * qk + qi * qj), 1.0f - 2.0f * (qj * qj + qk * qk)) * rad_to_deg;

	float bearing = (yaw < 0.0f) ? yaw + 360.0f : yaw;

	/* The BNO085's yaw=0 reference is 180 degrees from this board's Y+
	 * silkscreen direction */
	bearing = fmodf(bearing + 180.0f, 360.0f);

	/* Angle between the board's Z axis and vertical (0 = level) */
	float tilt = acosf(fmaxf(-1.0f, fminf(1.0f, 1.0f - 2.0f * (qi * qi + qj * qj)))) * rad_to_deg;

	char roll_s[16], pitch_s[16], yaw_s[16], bearing_s[16], tilt_s[16];
	format_degrees(roll_s, sizeof(roll_s), roll);
	format_degrees(pitch_s, sizeof(pitch_s), pitch);
	format_degrees(yaw_s, sizeof(yaw_s), yaw);
	format_degrees(bearing_s, sizeof(bearing_s), bearing);
	format_degrees(tilt_s, sizeof(tilt_s), tilt);

	char line[32];

	ssd1309_clear(oled);

	snprintf(line, sizeof(line), "Rotation Vector (%u/3)", accuracy);
	ssd1309_draw_string(oled, &font5x7, 0, 0, line, SSD1309_COLOR_ON);

	snprintf(line, sizeof(line), "Roll : %s", roll_s);
	ssd1309_draw_string(oled, &font5x7, 0, 10, line, SSD1309_COLOR_ON);

	snprintf(line, sizeof(line), "Pitch: %s", pitch_s);
	ssd1309_draw_string(oled, &font5x7, 0, 20, line, SSD1309_COLOR_ON);

	snprintf(line, sizeof(line), "Yaw  : %s", yaw_s);
	ssd1309_draw_string(oled, &font5x7, 0, 30, line, SSD1309_COLOR_ON);

	snprintf(line, sizeof(line), "Bearing: %s %s", bearing_s, compass_point(bearing));
	ssd1309_draw_string(oled, &font5x7, 0, 40, line, SSD1309_COLOR_ON);

	snprintf(line, sizeof(line), "Tilt : %s", tilt_s);
	ssd1309_draw_string(oled, &font5x7, 0, 50, line, SSD1309_COLOR_ON);

	ssd1309_flush(oled);
}

void test_bno085_game_rotation_vector_enable(bno085_t *p)
{
	bno085_drain(p);

	/* 100ms report interval (10Hz) */
	if (bno085_set_feature(p, BNO085_REPORT_GAME_ROTATION_VECTOR, 100000) != HAL_OK) {
		app_log("bno085: game rotation vector set feature failed\r\n");
	}
}

void test_bno085_game_rotation_vector_display(bno085_t *p, ssd1309_t *oled, uint32_t exec_us)
{
	uint8_t buf[32];
	uint16_t len = 0;

	if (bno085_read_packet(p, buf, sizeof(buf), &len) != HAL_OK) {
		return;
	}

	uint16_t print_len = (len < sizeof(buf)) ? len : (uint16_t)sizeof(buf);

	if (buf[2] != BNO085_CHANNEL_REPORTS) {
		return;
	}

	const uint8_t *payload = &buf[BNO085_SHTP_HEADER_SIZE];
	uint16_t available = (uint16_t)(print_len - BNO085_SHTP_HEADER_SIZE);
	uint16_t off = 0;

	if (off < available && payload[off] == BNO085_REPORT_BASE_TIMESTAMP_REFERENCE) {
		off += BNO085_BASE_TIMESTAMP_REFERENCE_SIZE;
	}

	if (!((uint16_t)(off + BNO085_GAME_ROTATION_VECTOR_SIZE) <= available && payload[off] == BNO085_REPORT_GAME_ROTATION_VECTOR)) {
		return;
	}

	const uint8_t *r = &payload[off];
	int16_t i_raw = (int16_t)(r[4] | (r[5] << 8));
	int16_t j_raw = (int16_t)(r[6] | (r[7] << 8));
	int16_t k_raw = (int16_t)(r[8] | (r[9] << 8));
	int16_t real_raw = (int16_t)(r[10] | (r[11] << 8));

	/* Q14 */
	float qi = (float)i_raw / 16384.0f;
	float qj = (float)j_raw / 16384.0f;
	float qk = (float)k_raw / 16384.0f;
	float qr = (float)real_raw / 16384.0f;

	const float rad_to_deg = 57.29578f;
	float roll  = atan2f(2.0f * (qr * qi + qj * qk), 1.0f - 2.0f * (qi * qi + qj * qj)) * rad_to_deg;
	float pitch = asinf(fmaxf(-1.0f, fminf(1.0f, 2.0f * (qr * qj - qk * qi)))) * rad_to_deg;
	float yaw   = atan2f(2.0f * (qr * qk + qi * qj), 1.0f - 2.0f * (qj * qj + qk * qk)) * rad_to_deg;

	char roll_s[16], pitch_s[16], yaw_s[16];
	format_degrees(roll_s, sizeof(roll_s), roll);
	format_degrees(pitch_s, sizeof(pitch_s), pitch);
	format_degrees(yaw_s, sizeof(yaw_s), yaw);

	char line[24];

	ssd1309_clear(oled);
	ssd1309_draw_string(oled, &font5x7, 0, 0, "Game Rotation Vector", SSD1309_COLOR_ON);

	snprintf(line, sizeof(line), "Roll : %s", roll_s);
	ssd1309_draw_string(oled, &font5x7, 0, 10, line, SSD1309_COLOR_ON);

	snprintf(line, sizeof(line), "Pitch: %s", pitch_s);
	ssd1309_draw_string(oled, &font5x7, 0, 20, line, SSD1309_COLOR_ON);

	snprintf(line, sizeof(line), "Yaw  : %s", yaw_s);
	ssd1309_draw_string(oled, &font5x7, 0, 30, line, SSD1309_COLOR_ON);

	if (exec_us > 0) {
		snprintf(line, sizeof(line), "Time : %lu us", (unsigned long)exec_us);
		ssd1309_draw_string(oled, &font5x7, 0, 40, line, SSD1309_COLOR_ON);
	}

	ssd1309_flush(oled);
}

void test_bno085_compass_enable(bno085_t *p)
{
	bno085_drain(p);

	/* Enable continuous calibration so the accuracy estimates can climb */
	if (bno085_set_calibration(p, true, true, true) != HAL_OK) {
		app_log("bno085: set calibration failed\r\n");
	}

	/* 100ms report interval (10Hz) for both reports */
	if (bno085_set_feature(p, BNO085_REPORT_ROTATION_VECTOR, 100000) != HAL_OK) {
		app_log("bno085: rotation vector set feature failed\r\n");
	}

	if (bno085_set_feature(p, BNO085_REPORT_MAGNETIC_FIELD_CALIBRATED, 100000) != HAL_OK) {
		app_log("bno085: magnetic field set feature failed\r\n");
	}
}

void test_bno085_compass_display(bno085_t *p, ssd1309_t *oled)
{
	static float bearing_deg = 0.0f;
	static float field_ut = 0.0f;
	static uint8_t accuracy = 0;
	static uint8_t mag_accuracy = 0;

	uint8_t buf[32];
	uint16_t len = 0;

	if (bno085_read_packet(p, buf, sizeof(buf), &len) != HAL_OK) {
		return;
	}

	uint16_t print_len = (len < sizeof(buf)) ? len : (uint16_t)sizeof(buf);

	if (buf[2] != BNO085_CHANNEL_REPORTS) {
		return;
	}

	const uint8_t *payload = &buf[BNO085_SHTP_HEADER_SIZE];
	uint16_t available = (uint16_t)(print_len - BNO085_SHTP_HEADER_SIZE);
	uint16_t off = 0;
	bool updated = false;

	while (off < available) {
		uint8_t report_id = payload[off];

		if (report_id == BNO085_REPORT_BASE_TIMESTAMP_REFERENCE
				&& (uint16_t)(off + BNO085_BASE_TIMESTAMP_REFERENCE_SIZE) <= available) {
			off += BNO085_BASE_TIMESTAMP_REFERENCE_SIZE;
		} else if (report_id == BNO085_REPORT_ROTATION_VECTOR
				&& (uint16_t)(off + BNO085_ROTATION_VECTOR_SIZE) <= available) {
			const uint8_t *r = &payload[off];
			int16_t k_raw = (int16_t)(r[8] | (r[9] << 8));
			int16_t real_raw = (int16_t)(r[10] | (r[11] << 8));

			/* Q14 */
			float qk = (float)k_raw / 16384.0f;
			float qr = (float)real_raw / 16384.0f;

			/* Yaw only (i/j not needed for a heading) */
			int16_t i_raw = (int16_t)(r[4] | (r[5] << 8));
			int16_t j_raw = (int16_t)(r[6] | (r[7] << 8));
			float qi = (float)i_raw / 16384.0f;
			float qj = (float)j_raw / 16384.0f;

			const float rad_to_deg = 57.29578f;
			float yaw = atan2f(2.0f * (qr * qk + qi * qj), 1.0f - 2.0f * (qj * qj + qk * qk)) * rad_to_deg;

			float bearing = (yaw < 0.0f) ? yaw + 360.0f : yaw;

			/* The BNO085's yaw=0 reference is 180 degrees from this board's
			 * Y+ silkscreen direction */
			bearing_deg = fmodf(bearing + 180.0f, 360.0f);
			accuracy = r[2] & 0x03u;
			updated = true;
			off += BNO085_ROTATION_VECTOR_SIZE;
		} else if (report_id == BNO085_REPORT_MAGNETIC_FIELD_CALIBRATED
				&& (uint16_t)(off + BNO085_MAGNETIC_FIELD_CALIBRATED_SIZE) <= available) {
			const uint8_t *r = &payload[off];
			int16_t x_raw = (int16_t)(r[4] | (r[5] << 8));
			int16_t y_raw = (int16_t)(r[6] | (r[7] << 8));
			int16_t z_raw = (int16_t)(r[8] | (r[9] << 8));

			/* Q4, uTesla */
			float x = (float)x_raw / 16.0f;
			float y = (float)y_raw / 16.0f;
			float z = (float)z_raw / 16.0f;

			field_ut = sqrtf(x * x + y * y + z * z);
			mag_accuracy = r[2] & 0x03u;
			updated = true;
			off += BNO085_MAGNETIC_FIELD_CALIBRATED_SIZE;
		} else {
			break;
		}
	}

	if (!updated) {
		return;
	}

	char bearing_s[16], field_s[16];
	format_degrees(bearing_s, sizeof(bearing_s), bearing_deg);
	format_degrees(field_s, sizeof(field_s), field_ut);

	char line[32];

	ssd1309_clear(oled);
	ssd1309_draw_string(oled, &font5x7, 0, 0, "Compass", SSD1309_COLOR_ON);

	snprintf(line, sizeof(line), "Bearing: %s %s", bearing_s, compass_point(bearing_deg));
	ssd1309_draw_string(oled, &font5x7, 0, 10, line, SSD1309_COLOR_ON);

	snprintf(line, sizeof(line), "Field : %s uT", field_s);
	ssd1309_draw_string(oled, &font5x7, 0, 20, line, SSD1309_COLOR_ON);

	snprintf(line, sizeof(line), "Acc: %u/3 (mag %u/3)", accuracy, mag_accuracy);
	ssd1309_draw_string(oled, &font5x7, 0, 30, line, SSD1309_COLOR_ON);

	ssd1309_flush(oled);
}

#endif /* TEST_BNO085 */
