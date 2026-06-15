#include "Tests/test_lsm6dsrx.h"
#include "app.h"

#ifdef TEST_LSM6DSRX

void test_lsm6dsrx_poll(lsm6dsrx_t *p)
{
	bool xl_ready = false;
	bool g_ready = false;

	if (lsm6dsrx_data_ready(p, &xl_ready, &g_ready) != HAL_OK) {
		app_log("lsm6dsrx: data_ready failed\r\n");
		return;
	}

	if (xl_ready) {
		int16_t x, y, z;
		if (lsm6dsrx_read_accel(p, &x, &y, &z) == HAL_OK) {
			app_log("lsm6dsrx: accel=%d,%d,%d\r\n", x, y, z);
		}
	}

	if (g_ready) {
		int16_t x, y, z;
		if (lsm6dsrx_read_gyro(p, &x, &y, &z) == HAL_OK) {
			app_log("lsm6dsrx: gyro=%d,%d,%d\r\n", x, y, z);
		}
	}
}

#endif /* TEST_LSM6DSRX */
