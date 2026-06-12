
#ifndef INC_TESTS_TEST_BNO085_H_
#define INC_TESTS_TEST_BNO085_H_

#include "bno085.h"
#include "ssd1309.h"

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

/**
 * test_bno085_rotation_vector
 * @param p - Pointer to an initialized bno085_t struct
 *
 * Sends a Set Feature Command (0xFD) to enable the Rotation Vector report
 * (0x05), then reads one report packet on the sensor reports channel and
 * logs the decoded quaternion (i, j, k, real) and accuracy estimate over
 * app_log(). Bench test for enabling and reading a sensor report.
 */
void test_bno085_rotation_vector(bno085_t *p);

/**
 * test_bno085_rotation_vector_enable
 * @param p - Pointer to an initialized bno085_t struct
 *
 * Drains any pending packets, then sends a Set Feature Command (0xFD) to
 * enable the Rotation Vector report (0x05) at a 100ms (10Hz) interval. Call
 * once during init; the device then streams reports on its own.
 */
void test_bno085_rotation_vector_enable(bno085_t *p);

/**
 * test_bno085_rotation_vector_display
 * @param p - Pointer to an initialized bno085_t struct
 * @param oled - Pointer to an initialized, brought-up ssd1309_t struct
 * @param exec_us - Execution time of the previous call, in microseconds, to
 *                   display alongside the orientation (0 to omit)
 *
 * Reads one SHTP packet (non-blocking, no drain or delay). If it contains a
 * Rotation Vector report (0x05) on the sensor reports channel, converts the
 * quaternion to roll/pitch/yaw (degrees) plus the status accuracy field, and
 * draws them on the OLED along with exec_us. Otherwise leaves the display
 * unchanged. Intended to be called periodically from app_loop() after
 * test_bno085_rotation_vector_enable() has been called once.
 */
void test_bno085_rotation_vector_display(bno085_t *p, ssd1309_t *oled, uint32_t exec_us);

/**
 * test_bno085_game_rotation_vector_enable
 * @param p - Pointer to an initialized bno085_t struct
 *
 * Drains any pending packets, then sends a Set Feature Command (0xFD) to
 * enable the Game Rotation Vector report (0x08) at a 100ms (10Hz) interval.
 * Call once during init; the device then streams reports on its own.
 */
void test_bno085_game_rotation_vector_enable(bno085_t *p);

/**
 * test_bno085_game_rotation_vector_display
 * @param p - Pointer to an initialized bno085_t struct
 * @param oled - Pointer to an initialized, brought-up ssd1309_t struct
 * @param exec_us - Execution time of the previous call, in microseconds, to
 *                   display alongside the orientation (0 to omit)
 *
 * Reads one SHTP packet (non-blocking, no drain or delay). If it contains a
 * Game Rotation Vector report (0x08) on the sensor reports channel, converts
 * the quaternion to roll/pitch/yaw (degrees) and draws them on the OLED
 * along with exec_us (no accuracy estimate - Game Rotation Vector has none).
 * Otherwise leaves the display unchanged. Intended to be called periodically
 * from app_loop() after test_bno085_game_rotation_vector_enable() has been
 * called once.
 */
void test_bno085_game_rotation_vector_display(bno085_t *p, ssd1309_t *oled, uint32_t exec_us);

/**
 * test_bno085_compass_enable
 * @param p - Pointer to an initialized bno085_t struct
 *
 * Drains any pending packets, sends a Configure ME Calibration command
 * (0xF2) to enable continuous accelerometer, gyro and magnetometer
 * calibration, then sends Set Feature Commands (0xFD) to enable both the
 * Rotation Vector report (0x05) and the Magnetic Field Calibrated report
 * (0x03), each at a 100ms (10Hz) interval. Call once during init; the
 * device then streams both reports on its own.
 */
void test_bno085_compass_enable(bno085_t *p);

/**
 * test_bno085_compass_display
 * @param p - Pointer to an initialized bno085_t struct
 * @param oled - Pointer to an initialized, brought-up ssd1309_t struct
 *
 * Reads one SHTP packet (non-blocking, no drain or delay). If it contains a
 * Rotation Vector report (0x05), converts the quaternion's yaw to a 0-360
 * degree bearing plus an 8-point compass label (N/NE/E/.../NW), and reads
 * its status accuracy estimate (0-3). If it contains a Magnetic Field
 * Calibrated report (0x03), computes the total field strength in uT as a
 * sanity check (Earth's field is roughly 25-65uT) and reads its own status
 * accuracy estimate (0-3), which reflects the magnetometer's calibration
 * confidence independently of the fused Rotation Vector accuracy. Both
 * reports may arrive in the same packet; whichever is present updates a
 * cached value, and the OLED is redrawn with the latest bearing, direction,
 * field strength and both accuracy estimates. Otherwise leaves the display
 * unchanged. Intended to be called
 * periodically from app_loop() after test_bno085_compass_enable() has been
 * called once.
 */
void test_bno085_compass_display(bno085_t *p, ssd1309_t *oled);

#endif /* TEST_BNO085 */

#endif /* INC_TESTS_TEST_BNO085_H_ */
