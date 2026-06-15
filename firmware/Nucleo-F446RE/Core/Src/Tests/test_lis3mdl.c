#include "Tests/test_lis3mdl.h"
#include "app.h"

#ifdef TEST_LIS3MDL

void test_lis3mdl_poll(lis3mdl_t *p)
{
	bool ready;

	if (lis3mdl_data_ready(p, &ready) != HAL_OK || !ready) {
		return;
	}

	int16_t x, y, z;
	if (lis3mdl_read_mag(p, &x, &y, &z) != HAL_OK) {
		return;
	}

	app_log("lis3mdl: mag=%d,%d,%d\r\n", x, y, z);
}

#endif /* TEST_LIS3MDL */
