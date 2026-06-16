/*
 * ota_rx.h -- RTCM3 over-the-air reassembly module (rover RX side)
 *
 * Receives fixed-size SX1262 GFSK packets from the base, reassembles them
 * into complete RTCM3 frames, and fires a caller-supplied callback for each
 * complete frame.
 *
 * ---------------------------------------------------------------------------
 * OTA PACKET FORMAT  (must match ota.h on the base side)
 * ---------------------------------------------------------------------------
 *
 *   Byte 0   type       0x01 = RTCM3 chunk (other values ignored)
 *   Byte 1   seq        frame sequence number, 0-255 wrapping
 *   Byte 2   chunk_idx  0-based index of this chunk within the frame
 *   Byte 3   chunk_count total number of chunks for this frame
 *   Byte 4   data_len   number of valid RTCM3 bytes in data[] (1-250)
 *   Bytes 5-254  data   RTCM3 frame bytes (last chunk zero-padded to fill)
 *
 * ---------------------------------------------------------------------------
 * REASSEMBLY RULES
 * ---------------------------------------------------------------------------
 * A frame is started when a packet with chunk_idx=0 arrives. Subsequent
 * chunks are accepted only if seq and chunk_idx match what is expected.
 * Any gap (dropped packet, wrong seq mid-frame) abandons the in-progress
 * frame silently; reassembly restarts on the next chunk_idx=0 packet.
 *
 * ---------------------------------------------------------------------------
 * INTEGRATION STEPS
 * ---------------------------------------------------------------------------
 *
 * 1. CALLER OWNS THE INSTANCE -- define at file scope in app.c:
 *
 *        static ota_rx_t ota_rx;
 *
 * 2. INITIALISE -- call once from app_init(), after sx1262_config_gfsk():
 *
 *        ota_rx_init(&ota_rx, &sx1262, on_rtcm3_frame);
 *
 *    ota_rx_init() registers its own rx_done callback via
 *    sx1262_set_rx_done_callback(). Do not register a separate rx_done
 *    callback after this.
 *
 * 3. ARM RX -- call once before app_loop() starts, and again after each
 *    DIO1 service:
 *
 *        sx1262_set_rx(&sx1262, SX1262_RX_TIMEOUT_NONE);
 *
 * 4. SERVICE IN LOOP -- when flag_get_SX1262_DIO1() fires:
 *
 *        sx1262_service_rx(&sx1262);      // fires ota_rx's rx_done
 *        sx1262_set_rx(&sx1262, 0);       // re-arm for next packet
 */

#ifndef INC_OTA_RX_H_
#define INC_OTA_RX_H_

#include <stdint.h>
#include <stdbool.h>
#include "sx1262.h"

#define OTA_RX_PACKET_SIZE    255
#define OTA_RX_HEADER_SIZE    5
#define OTA_RX_DATA_SIZE      (OTA_RX_PACKET_SIZE - OTA_RX_HEADER_SIZE)  /* 250 */
#define OTA_RX_FRAME_BUF_SIZE 1032   /* max RTCM3 frame is 1029 bytes, rounded up */

#define OTA_RX_TYPE_RTCM3     0x01

typedef struct {
	sx1262_t *sx1262;
	void     (*on_frame)(const uint8_t *frame, uint16_t len);
	uint8_t    frame_buf[OTA_RX_FRAME_BUF_SIZE];
	uint16_t   frame_pos;
	uint8_t    expected_seq;
	uint8_t    expected_chunk_idx;
	uint8_t    chunk_count;
	bool       in_progress;
} ota_rx_t;

void ota_rx_init(ota_rx_t *p, sx1262_t *sx1262,
                 void (*on_frame)(const uint8_t *frame, uint16_t len));

#endif /* INC_OTA_RX_H_ */
