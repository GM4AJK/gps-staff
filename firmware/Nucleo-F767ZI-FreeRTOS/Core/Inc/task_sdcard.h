#pragma once
#include <stdint.h>

/* Maximum RTCM3 frame size logged: protocol max is 1023-byte payload + 6 framing = 1029. */
#define SD_FRAME_MAX  1032

void task_sdcard_init(void);
void task_sdcard_push_frame(const uint8_t *data, uint16_t len);
