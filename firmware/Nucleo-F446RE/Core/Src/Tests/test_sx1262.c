#include "Tests/test_sx1262.h"
#include "app.h"

#ifdef TEST_SX1262

void test_sx1262_hello(sx1262_t *p)
{
	HAL_StatusTypeDef status;
	uint8_t value;

	status = sx1262_reset(p);
	if (status != HAL_OK) {
		app_log("sx1262: reset failed: %d\r\n", status);
		return;
	}

	status = sx1262_get_status(p, &value);
	if (status != HAL_OK) {
		app_log("sx1262: get status failed: %d\r\n", status);
		return;
	}

	uint8_t chip_mode = (value >> 4) & 0x07;
	uint8_t cmd_status = (value >> 1) & 0x07;

	app_log("sx1262: status=0x%02X (chip mode=%u, cmd status=%u)\r\n", value, chip_mode, cmd_status);
}

#endif /* TEST_SX1262 */
