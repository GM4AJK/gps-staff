#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "ota_rx.h"
#include "sx1262.h"

static ota_rx_t *ota_rx_instance;

static void on_rx_done(sx1262_t *sx,
                       const uint8_t *payload, size_t len,
                       int8_t rssi, int8_t snr_quarter_db)
{
	(void)sx;

	ota_rx_t *p = ota_rx_instance;
	p->last_rssi           = rssi;
	p->last_snr_quarter_db = snr_quarter_db;

	if(len < OTA_RX_PACKET_SIZE) {
		return;
	}

	uint8_t type        = payload[0];
	uint8_t seq         = payload[1];
	uint8_t chunk_idx   = payload[2];
	uint8_t chunk_count = payload[3];
	uint8_t data_len    = payload[4];

	if(type != OTA_RX_TYPE_RTCM3) {
		return;
	}

	if(data_len == 0 || data_len > OTA_RX_DATA_SIZE) {
		return;
	}

	if(chunk_idx == 0) {
		p->in_progress        = true;
		p->expected_seq       = seq;
		p->chunk_count        = chunk_count;
		p->expected_chunk_idx = 0;
		p->frame_pos          = 0;
	} else {
		if(!p->in_progress || seq != p->expected_seq || chunk_idx != p->expected_chunk_idx) {
			p->in_progress = false;
			return;
		}
	}

	if((uint32_t)p->frame_pos + data_len > OTA_RX_FRAME_BUF_SIZE) {
		p->in_progress = false;
		return;
	}

	memcpy(&p->frame_buf[p->frame_pos], &payload[OTA_RX_HEADER_SIZE], data_len);
	p->frame_pos += data_len;
	p->expected_chunk_idx++;

	if(chunk_idx + 1 == chunk_count) {
		if(p->on_frame) {
			p->on_frame(p->frame_buf, p->frame_pos);
		}
		p->in_progress = false;
	}
}

void ota_rx_init(ota_rx_t *p, sx1262_t *sx1262,
                 void (*on_frame)(const uint8_t *frame, uint16_t len))
{
	ota_rx_instance       = p;
	p->sx1262             = sx1262;
	p->on_frame           = on_frame;
	p->frame_pos          = 0;
	p->expected_seq       = 0;
	p->expected_chunk_idx = 0;
	p->chunk_count        = 0;
	p->in_progress        = false;

	memset(p->frame_buf, 0, sizeof(p->frame_buf));

	sx1262_set_rx_done_callback(sx1262, on_rx_done);
}
