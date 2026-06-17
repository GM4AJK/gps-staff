#include "Tests/test_motioncal.h"

#ifdef TEST_MOTIONCAL

#include "app.h"

void test_motioncal_init(lis3mdl_t *mag)
{
	if(lis3mdl_bringup(mag) != HAL_OK)
		app_log("test_motioncal: lis3mdl_bringup failed\r\n");
}

HAL_StatusTypeDef test_motioncal_poll(lis3mdl_t *mag)
{
	bool ready = false;
	HAL_StatusTypeDef status = lis3mdl_data_ready(mag, &ready);
	if(status != HAL_OK) return status;
	if(!ready) return HAL_OK;

	int16_t mx, my, mz;
	status = lis3mdl_read_mag(mag, &mx, &my, &mz);
	if(status != HAL_OK) return status;

	/* MotionCal ASCII format: accel(3) + gyro(3) + mag(3), all int16_t */
	app_log("Raw: 0,0,0,0,0,0,%d,%d,%d\r\n", (int)mx, (int)my, (int)mz);
	return HAL_OK;
}

#endif /* TEST_MOTIONCAL */
