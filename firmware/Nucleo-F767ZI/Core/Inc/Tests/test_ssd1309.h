
#ifndef INC_TESTS_TEST_SSD1309_H_
#define INC_TESTS_TEST_SSD1309_H_

#include "ssd1309.h"

/* Comment out to exclude the ssd1309 bench test from the build */
#define TEST_SSD1309

#ifdef TEST_SSD1309

/**
 * test_ssd1309_shapes
 * @param p - Pointer to an initialized ssd1309_t struct, after a
 *            successful ssd1309_bringup()
 *
 * Draws a fixed set of text, lines and shapes to the framebuffer and
 * flushes it to the display. Bench smoke test for the ssd1309 driver's
 * drawing primitives.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed
 *         ssd1309_flush().
 */
HAL_StatusTypeDef test_ssd1309_shapes(ssd1309_t *p);

#endif /* TEST_SSD1309 */

#endif /* INC_TESTS_TEST_SSD1309_H_ */
