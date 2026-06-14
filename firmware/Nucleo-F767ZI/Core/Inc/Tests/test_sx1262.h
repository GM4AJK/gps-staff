#ifndef INC_TESTS_TEST_SX1262_H_
#define INC_TESTS_TEST_SX1262_H_

#include "sx1262.h"

/* Comment out to exclude the sx1262 bench test from the build */
#define TEST_SX1262

#ifdef TEST_SX1262

/**
 * test_sx1262_hello
 * @param p - Pointer to an initialized sx1262_t struct
 *
 * Resets the SX1262, then sends a GetStatus (0xC0) command over SPI and
 * logs the result and decoded status byte over app_log(). Bench smoke test
 * for SPI2 framing, NSS and BUSY handling.
 */
void test_sx1262_hello(sx1262_t *p);

#endif /* TEST_SX1262 */

#endif /* INC_TESTS_TEST_SX1262_H_ */
