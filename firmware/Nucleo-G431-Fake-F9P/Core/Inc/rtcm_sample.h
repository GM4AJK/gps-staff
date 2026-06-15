#ifndef INC_RTCM_SAMPLE_H_
#define INC_RTCM_SAMPLE_H_

#include <stdint.h>

/*
 * RTCM3 sample captured from the CambridgeSensoriis RTK2GO mount
 * (see firmware/data/CambridgeSensoriisSample.bin), trimmed to
 * 12 complete 1005/1074/1084 (GPS+GLONASS MSM4) cycles
 * for cyclic playback as a fake F9P base output.
 */

extern const uint8_t rtcm_sample_data[2514];
extern const uint32_t rtcm_sample_len;
extern const uint32_t rtcm_sample_cycle_offsets[12];
extern const uint32_t rtcm_sample_cycle_count;

#endif /* INC_RTCM_SAMPLE_H_ */
