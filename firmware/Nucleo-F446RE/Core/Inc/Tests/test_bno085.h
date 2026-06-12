
#ifndef INC_TESTS_TEST_BNO085_H_
#define INC_TESTS_TEST_BNO085_H_

#include "bno085.h"

/* Comment out to exclude the bno085 bench test from the build */
#define TEST_BNO085

#ifdef TEST_BNO085

/**
 * test_bno085_hello
 * @param p - Pointer to an initialized bno085_t struct
 *
 * Probes the BNO085 over I2C and, if it acknowledges, reads and logs one
 * SHTP packet (length, channel, sequence number and a hex dump) over
 * app_log(). Bench smoke test for I2C wiring and basic SHTP framing.
 */
void test_bno085_hello(bno085_t *p);

/**
 * test_bno085_product_id
 * @param p - Pointer to an initialized bno085_t struct
 *
 * Sends a Product ID Request (0xF9) on the SH-2 control channel, then reads
 * and logs the response SHTP packet (length, channel, sequence number and a
 * hex dump) over app_log(). Bench test for a two-way SH-2 request/response.
 */
void test_bno085_product_id(bno085_t *p);

#endif /* TEST_BNO085 */

#endif /* INC_TESTS_TEST_BNO085_H_ */
