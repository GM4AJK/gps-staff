#ifndef INC_TESTS_TEST_LIS3MDL_H_
#define INC_TESTS_TEST_LIS3MDL_H_

#include "lis3mdl.h"

/* Comment out to exclude the lis3mdl bench test from the build */
#define TEST_LIS3MDL

#ifdef TEST_LIS3MDL

/**
 * test_lis3mdl_poll
 * @param p - Pointer to an initialized lis3mdl_t struct
 *
 * Checks STATUS_REG via lis3mdl_data_ready(); if a new X/Y/Z data set is
 * available, reads it via lis3mdl_read_mag() and logs the raw values
 * over app_log(). Bench verification only - not intended for production
 * use.
 */
void test_lis3mdl_poll(lis3mdl_t *p);

#endif /* TEST_LIS3MDL */

#endif /* INC_TESTS_TEST_LIS3MDL_H_ */
