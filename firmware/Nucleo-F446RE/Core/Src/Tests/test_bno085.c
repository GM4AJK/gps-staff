
#include "Tests/test_bno085.h"
#include "app.h"
#include <stdio.h>
#include <stdbool.h>

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

#endif /* TEST_BNO085 */
