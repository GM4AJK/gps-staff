#ifndef INC_RTCM_SAMPLE_H_
#define INC_RTCM_SAMPLE_H_

#include <stdint.h>

/*
 * RTCM3 sample for Fake-F9P-Base canned playback.
 *
 * Each cycle contains 5 messages:
 *   1005  Station ARP            25 bytes    1 chunk
 *   1074  GPS MSM4              120 bytes    1 chunk   (real capture, CambridgeSensoriis RTK2GO)
 *   1084  GLONASS MSM4           69 bytes    1 chunk   (real capture)
 *   1094  Galileo MSM4 (synth)  300 bytes    2 chunks  (valid CRC, zeroed obs data)
 *   1124  BeiDou  MSM4 (synth)  550 bytes    3 chunks  (valid CRC, zeroed obs data)
 *
 * The 1094/1124 frames are synthetic: correct RTCM3 framing + CRC24Q,
 * message type in the first 12 payload bits, observation data zeroed.
 * They exercise the OTA chunker without requiring real GNSS hardware.
 *
 * 6 cycles, 1064 bytes each, 6384 bytes total.
 */

extern const uint8_t  rtcm_sample_data[6384];
extern const uint32_t rtcm_sample_len;
extern const uint32_t rtcm_sample_cycle_offsets[6];
extern const uint32_t rtcm_sample_cycle_count;

#endif /* INC_RTCM_SAMPLE_H_ */
