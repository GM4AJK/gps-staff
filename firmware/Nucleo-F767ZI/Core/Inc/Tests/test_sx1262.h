#ifndef INC_TESTS_TEST_SX1262_H_
#define INC_TESTS_TEST_SX1262_H_

#include "sx1262.h"
#include "ssd1309.h"
#include <stdbool.h>

/* Comment out to exclude the sx1262 bench test from the build */
#define TEST_SX1262

#ifdef TEST_SX1262

/**
 * test_sx1262_set_oled
 * @param p - Pointer to an initialized ssd1309_t struct, or NULL to
 *            disable the OLED update in test_sx1262_rx_done_handler()
 *
 * Registers the display used by test_sx1262_rx_done_handler() to show
 * the last received packet's payload, RSSI and SNR. NULL until set.
 */
void test_sx1262_set_oled(ssd1309_t *p);

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
 * test_sx1262_tx_start
 * @param p - Pointer to an initialized sx1262_t struct
 *
 * Writes an 8-byte "PING000x" payload (x = rotating counter digit) to
 * the radio's TX buffer and triggers a single SetTx. Returns
 * immediately; completion is signalled by the DIO1 IRQ and handled by
 * test_sx1262_tx_done().
 */
void test_sx1262_tx_start(sx1262_t *p);

/**
 * test_sx1262_tx_done
 * @param p - Pointer to an initialized sx1262_t struct
 *
 * Called once the DIO1 IRQ fires for a pending TX. Reads GetIrqStatus,
 * logs TxDone/Timeout over app_log() and clears the IRQ flags. Returns
 * true if the IRQ was TxDone, false on timeout.
 */
bool test_sx1262_tx_done(sx1262_t *p);

/**
 * test_sx1262_rx_start
 * @param p - Pointer to an initialized sx1262_t struct
 *
 * Triggers a single SetRx with a ~1s radio timeout. Returns
 * immediately; completion is signalled by the DIO1 IRQ and handled by
 * test_sx1262_rx_done().
 */
void test_sx1262_rx_start(sx1262_t *p);

/**
 * test_sx1262_rx_done
 * @param p - Pointer to an initialized sx1262_t struct
 *
 * Called once the DIO1 IRQ fires for a pending RX. Reads GetIrqStatus;
 * on RxDone reads back the 8-byte payload via ReadBuffer. Logs the
 * result over app_log() and clears the IRQ flags. Returns true if the
 * IRQ was RxDone (a packet was actually received), false on timeout.
 */
bool test_sx1262_rx_done(sx1262_t *p);

/**
 * test_sx1262_rx_done_handler
 * @param p - Pointer to the sx1262_t instance that received the packet
 *
 * Toggles LD2, then (if an OLED has been registered via
 * test_sx1262_set_oled()) clears the display and shows the last received
 * packet's payload, RSSI and SNR. Registered via
 * sx1262_set_rx_done_callback() in test_sx1262_config() - called from
 * test_sx1262_rx_done() whenever a packet is actually received.
 */
void test_sx1262_rx_done_handler(sx1262_t *p);

/**
 * test_sx1262_tx_done_toggle_led
 * @param p - Pointer to the sx1262_t instance that completed transmission
 *
 * Toggles LD2. Registered via sx1262_set_tx_done_callback() in
 * test_sx1262_config() as a demo of the tx_done callback - called from
 * test_sx1262_tx_done() whenever a transmission actually completes.
 */
void test_sx1262_tx_done_toggle_led(sx1262_t *p);

#endif /* TEST_SX1262 */

#endif /* INC_TESTS_TEST_SX1262_H_ */
