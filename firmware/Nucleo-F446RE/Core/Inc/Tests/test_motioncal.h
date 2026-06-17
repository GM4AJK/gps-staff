#ifndef INC_TESTS_TEST_MOTIONCAL_H_
#define INC_TESTS_TEST_MOTIONCAL_H_

#include "lis3mdl.h"

/* Comment out to exclude the MotionCal streaming test from the build */
#define TEST_MOTIONCAL

#ifdef TEST_MOTIONCAL

/**
 * test_motioncal_init
 * @param mag - Pointer to an initialized lis3mdl_t struct
 *
 * Calls lis3mdl_bringup() and logs the result.  Call once from
 * app_init() after the I2C peripheral is ready.
 */
void test_motioncal_init(lis3mdl_t *mag);

/**
 * test_motioncal_poll
 * @param mag - Pointer to a brought-up lis3mdl_t struct
 *
 * If new magnetometer data is ready, emits one MotionCal ASCII line:
 *   Raw: 0,0,0,0,0,0,mx,my,mz\r\n
 * Accel and gyro fields are zero (not available); MotionCal uses only
 * the mag values for magnetic calibration.  Call at the LIS3MDL's ODR
 * (10 Hz → every 100 ms) from app_loop().
 *
 * Point MotionCal at the F446RE COM port, rotate the board through as
 * many orientations as possible until the fit-error numbers plateau,
 * then read off V[] (hard-iron) and invW[][] (soft-iron matrix).
 */
void test_motioncal_poll(lis3mdl_t *mag);

#endif /* TEST_MOTIONCAL */

#endif /* INC_TESTS_TEST_MOTIONCAL_H_ */
