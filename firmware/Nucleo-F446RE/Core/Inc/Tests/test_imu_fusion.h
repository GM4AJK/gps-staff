#ifndef INC_TESTS_TEST_IMU_FUSION_H_
#define INC_TESTS_TEST_IMU_FUSION_H_

#include "lsm6dsrx.h"
#include "lis3mdl.h"
#include "ssd1309.h"

/* Comment out to exclude the IMU+mag fusion bench test from the build */
/* #define TEST_IMU_FUSION */

#ifdef TEST_IMU_FUSION

/**
 * test_imu_fusion_set_oled
 * @param p - Pointer to an initialized ssd1309_t struct, or NULL to
 *            disable the OLED update in test_imu_fusion_poll()
 *
 * Registers the display used by test_imu_fusion_poll() to show the
 * current roll/pitch/heading. NULL until set.
 */
void test_imu_fusion_set_oled(ssd1309_t *p);

/**
 * test_imu_fusion_poll
 * @param imu - Pointer to an initialized lsm6dsrx_t struct
 * @param mag - Pointer to an initialized lis3mdl_t struct
 *
 * Reads the current accelerometer and magnetometer outputs, computes
 * roll/pitch via lsm6dsrx_compute_tilt() and a tilt-compensated heading
 * via lis3mdl_compute_heading(), and logs the result over app_log(). If
 * an OLED has been registered via test_imu_fusion_set_oled(), also
 * clears the display and shows the three values. Bench verification
 * only - not intended for production use.
 */
void test_imu_fusion_poll(lsm6dsrx_t *imu, lis3mdl_t *mag);

#endif /* TEST_IMU_FUSION */

#endif /* INC_TESTS_TEST_IMU_FUSION_H_ */
