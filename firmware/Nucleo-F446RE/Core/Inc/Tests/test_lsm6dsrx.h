#ifndef INC_TESTS_TEST_LSM6DSRX_H_
#define INC_TESTS_TEST_LSM6DSRX_H_

#include "lsm6dsrx.h"

/* Comment out to exclude the lsm6dsrx bench test from the build */
#define TEST_LSM6DSRX

#ifdef TEST_LSM6DSRX

/**
 * test_lsm6dsrx_poll
 * @param p - Pointer to an initialized lsm6dsrx_t struct
 *
 * Checks STATUS_REG for new accelerometer/gyroscope data and, if
 * available, reads the raw OUTX/Y/Z registers and logs them over
 * app_log(). Intended to be called periodically (e.g. every 500ms)
 * from app_loop().
 */
void test_lsm6dsrx_poll(lsm6dsrx_t *p);

#endif /* TEST_LSM6DSRX */

#endif /* INC_TESTS_TEST_LSM6DSRX_H_ */
