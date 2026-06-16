/*
 * ota_tx.h -- RTCM3 over-the-air chunking module (base TX side)
 *
 * Takes a complete RTCM3 frame, splits it into fixed-size SX1262 packets,
 * and sends them sequentially over the radio link. Each packet carries a
 * small header so the rover can identify, sequence, and reassemble frames.
 *
 * ---------------------------------------------------------------------------
 * OTA PACKET FORMAT  (OTA_TX_PACKET_SIZE = 255 bytes, fixed, matching the
 *                     SX1262 GFSK fixed-length configuration)
 * ---------------------------------------------------------------------------
 *
 *   Byte 0   type       0x01 = RTCM3 chunk (other values reserved)
 *   Byte 1   seq        frame sequence number, 0-255 wrapping; increments
 *                       once per complete RTCM3 frame handed to ota_tx_push_frame()
 *   Byte 2   chunk_idx  0-based index of this chunk within the frame
 *   Byte 3   chunk_count total number of chunks for this frame
 *   Byte 4   data_len   number of valid RTCM3 bytes in data[] (1-OTA_TX_DATA_SIZE);
 *                       the last chunk may be short, padded with zeros to fill
 *                       the fixed packet length
 *   Bytes 5-254  data   RTCM3 frame bytes (OTA_TX_DATA_SIZE = 250 bytes per chunk)
 *
 * ---------------------------------------------------------------------------
 * MEMORY MODEL
 * ---------------------------------------------------------------------------
 * ota_tx_push_frame() performs a memcpy of the incoming frame into ota_tx_t.frame_buf,
 * so rtcm3 can release its buffer immediately after the call returns. The two
 * modules share no memory.
 *
 * Up to OTA_TX_QUEUE_DEPTH frames may be queued while a TX is in progress. If
 * ota_tx_push_frame() is called when the queue is full, it returns false and the
 * frame is dropped. With a depth of 16, a full F9P epoch fits with comfortable
 * headroom: the first starts immediately, the remaining fifteen queue.
 *
 * ---------------------------------------------------------------------------
 * INTEGRATION STEPS
 * ---------------------------------------------------------------------------
 *
 * 1. CALLER OWNS THE INSTANCE -- define at file scope in app.c:
 *
 *        ota_tx_t ota_tx;
 *
 * 2. INITIALISE -- call once from app_init(), after sx1262 is configured:
 *
 *        ota_tx_init(&ota_tx, &sx1262);
 *
 *    ota_tx_init() registers on_tx_done() as the sx1262 TX-done callback
 *    via sx1262_set_tx_done_callback().
 *
 * 3. PUSH FRAMES -- call from rtcm3_loop() (or app_loop()) once rtcm3
 *    signals a complete frame is ready:
 *
 *        ota_tx_push_frame(&ota_tx, frame_ptr, frame_len);
 *
 * 4. SERVICE TX DONE -- in app_loop(), when flag_get_SX1262_DIO1() fires,
 *    call sx1262_service_tx() as normal; it will invoke the registered
 *    on_tx_done() callback, which queues the next chunk automatically.
 *    No additional wiring needed beyond the existing DIO1 path.
 */

#ifndef INC_OTA_TX_H_
#define INC_OTA_TX_H_

#include <stdint.h>
#include <stdbool.h>
#include "sx1262.h"

#define OTA_TX_PACKET_SIZE    255
#define OTA_TX_HEADER_SIZE    5
#define OTA_TX_DATA_SIZE      (OTA_TX_PACKET_SIZE - OTA_TX_HEADER_SIZE)  /* 250 */
#define OTA_TX_FRAME_BUF_SIZE 1032   /* max RTCM3 frame is 1029 bytes, rounded up */
#define OTA_TX_QUEUE_DEPTH    16     /* slots behind the active frame; covers a full F9P epoch with headroom */

#define OTA_TX_TYPE_RTCM3    0x01

typedef struct {
	uint8_t  buf[OTA_TX_FRAME_BUF_SIZE];
	uint16_t len;
} ota_tx_queue_entry_t;

typedef struct {
	sx1262_t             *sx1262;
	uint8_t               frame_buf[OTA_TX_FRAME_BUF_SIZE];
	uint16_t              frame_len;
	uint8_t               frame_seq;
	uint8_t               chunk_idx;
	uint8_t               chunk_count;
	bool                  busy;
	uint8_t               tx_buf[OTA_TX_PACKET_SIZE];
	ota_tx_queue_entry_t  queue[OTA_TX_QUEUE_DEPTH];
	uint8_t               queue_head;
	uint8_t               queue_tail;
	uint8_t               queue_count;
} ota_tx_t;

void ota_tx_init(ota_tx_t *p, sx1262_t *sx1262);
bool ota_tx_push_frame(ota_tx_t *p, const uint8_t *frame, uint16_t len);

#endif /* INC_OTA_TX_H_ */
