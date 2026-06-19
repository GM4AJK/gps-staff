#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "ota_tx.h"
#include "sx1262.h"

static ota_tx_t *ota_tx_instance;

static void ota_tx_send_next_chunk(ota_tx_t *p)
{
	uint16_t offset    = (uint16_t)p->chunk_idx * OTA_TX_DATA_SIZE;
	uint16_t remaining = p->frame_len - offset;
	uint8_t  data_len  = (remaining > OTA_TX_DATA_SIZE) ? OTA_TX_DATA_SIZE : (uint8_t)remaining;

	p->tx_buf[0] = OTA_TX_TYPE_RTCM3;
	p->tx_buf[1] = p->frame_seq;
	p->tx_buf[2] = p->chunk_idx;
	p->tx_buf[3] = p->chunk_count;
	p->tx_buf[4] = data_len;

	memcpy(&p->tx_buf[5], &p->frame_buf[offset], data_len);

	if(data_len < OTA_TX_DATA_SIZE) {
		memset(&p->tx_buf[5 + data_len], 0, OTA_TX_DATA_SIZE - data_len);
	}

	sx1262_write_buffer(p->sx1262, 0, p->tx_buf, OTA_TX_PACKET_SIZE);
	sx1262_set_tx(p->sx1262, 0);
}

static void ota_tx_start_frame(ota_tx_t *p, const uint8_t *frame, uint16_t len)
{
	memcpy(p->frame_buf, frame, len);
	p->frame_len   = len;
	p->chunk_idx   = 0;
	p->chunk_count = (uint8_t)((len + OTA_TX_DATA_SIZE - 1) / OTA_TX_DATA_SIZE);
	p->busy        = true;
	ota_tx_send_next_chunk(p);
}

static void on_tx_done(sx1262_t *sx)
{
	(void)sx;
	ota_tx_t *p = ota_tx_instance;

	p->chunk_idx++;

	if(p->chunk_idx < p->chunk_count) {
		ota_tx_send_next_chunk(p);
		return;
	}

	p->frame_seq++;
	p->busy = false;

	if(p->queue_count > 0) {
		ota_tx_queue_entry_t *e = &p->queue[p->queue_head];
		p->queue_head = (p->queue_head + 1) % OTA_TX_QUEUE_DEPTH;
		p->queue_count--;
		ota_tx_start_frame(p, e->buf, e->len);
	}
}

void ota_tx_init(ota_tx_t *p, sx1262_t *sx1262)
{
	ota_tx_instance = p;
	p->sx1262      = sx1262;
	p->frame_len   = 0;
	p->frame_seq   = 0;
	p->chunk_idx   = 0;
	p->chunk_count = 0;
	p->busy        = false;
	p->queue_head  = 0;
	p->queue_tail  = 0;
	p->queue_count = 0;

	memset(p->frame_buf, 0, sizeof(p->frame_buf));
	memset(p->tx_buf,    0, sizeof(p->tx_buf));
	memset(p->queue,     0, sizeof(p->queue));

	sx1262_set_tx_done_callback(sx1262, on_tx_done);
}

bool ota_tx_push_frame(ota_tx_t *p, const uint8_t *frame, uint16_t len)
{
	if(!p->busy) {
		ota_tx_start_frame(p, frame, len);
		return true;
	}

	if(p->queue_count >= OTA_TX_QUEUE_DEPTH) {
		return false;
	}

	ota_tx_queue_entry_t *e = &p->queue[p->queue_tail];
	p->queue_tail = (p->queue_tail + 1) % OTA_TX_QUEUE_DEPTH;
	p->queue_count++;
	memcpy(e->buf, frame, len);
	e->len = len;
	return true;
}
