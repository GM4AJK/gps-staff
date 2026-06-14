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

/**
 * test_sx1262_config
 * @param p - Pointer to an initialized sx1262_t struct
 *
 * Resets the SX1262, then sends SetPacketType (LoRa) followed by
 * SetRfFrequency (434.000MHz), SetModulationParams, SetPacketParams,
 * SetBufferBaseAddress, SetPaConfig and SetTxParams, logging the result
 * of each step over app_log(). Bench test for basic LoRa radio
 * configuration. Intended to be run once at startup.
 */
void test_sx1262_config(sx1262_t *p);

/**
 * test_sx1262_tx
 * @param p - Pointer to an initialized sx1262_t struct
 *
 * Writes an 8-byte "PING000x" payload (x = rotating counter digit) to
 * the radio's TX buffer and triggers a single SetTx. Polls
 * GetIrqStatus for TxDone/Timeout (up to 2s) before logging the result
 * over app_log() and clearing the IRQ flags.
 */
void test_sx1262_tx(sx1262_t *p);

/**
 * test_sx1262_rx
 * @param p - Pointer to an initialized sx1262_t struct
 *
 * Triggers a single SetRx with a ~1s radio timeout. Polls GetIrqStatus
 * for RxDone/Timeout (up to 1.1s); on RxDone reads back the 8-byte
 * payload via ReadBuffer. Logs the result over app_log() and clears the
 * IRQ flags.
 */
void test_sx1262_rx(sx1262_t *p);

#endif /* TEST_SX1262 */

#endif /* INC_TESTS_TEST_SX1262_H_ */
