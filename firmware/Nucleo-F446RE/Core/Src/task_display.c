#include "task_display.h"

#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "main.h"
#include "ssd130x.h"

#define DISPLAY_TEXT_LEN  64
#define DISPLAY_QUEUE_LEN  8

typedef enum {
	DISPLAY_CLEAR,
	DISPLAY_STRING,
	DISPLAY_FLUSH,
} display_msg_type_t;

typedef struct {
	display_msg_type_t type;
	union {
		struct {
			int16_t x;
			int16_t y;
			const ssd130x_font_t *font;
			char text[DISPLAY_TEXT_LEN];
		} string;
	};
} display_msg_t;

extern SemaphoreHandle_t hi2c1_mutex;

static QueueHandle_t display_queue;
static ssd130x_t oled;

static void display_task(void *arg)
{
	(void)arg;

	ssd130x_init(&oled, &hi2c1, 0x3C, SSD130X_CHIP_SSD1306, true, -1, -1);
	xSemaphoreTake(hi2c1_mutex, portMAX_DELAY);
	ssd130x_bringup(&oled);
	xSemaphoreGive(hi2c1_mutex);

	display_msg_t msg;
	for (;;) {
		if (xQueueReceive(display_queue, &msg, portMAX_DELAY) != pdTRUE)
			continue;

		switch (msg.type) {
		case DISPLAY_CLEAR:
			ssd130x_clear(&oled);
			break;
		case DISPLAY_STRING:
			ssd130x_draw_string(&oled, msg.string.font,
				msg.string.x, msg.string.y,
				msg.string.text, SSD130X_COLOR_ON);
			break;
		case DISPLAY_FLUSH:
			xSemaphoreTake(hi2c1_mutex, portMAX_DELAY);
			ssd130x_flush(&oled);
			xSemaphoreGive(hi2c1_mutex);
			break;
		}
	}
}

void display_init(void)
{
	display_queue = xQueueCreate(DISPLAY_QUEUE_LEN, sizeof(display_msg_t));
	configASSERT(display_queue);
	xTaskCreate(display_task, "display", 512, NULL, tskIDLE_PRIORITY + 1, NULL);
}

void display_clear(void)
{
	display_msg_t msg = { .type = DISPLAY_CLEAR };
	xQueueSend(display_queue, &msg, 0);
}

void display_string(int16_t x, int16_t y, const ssd130x_font_t *font, const char *str)
{
	display_msg_t msg = { .type = DISPLAY_STRING };
	msg.string.x    = x;
	msg.string.y    = y;
	msg.string.font = font;
	strncpy(msg.string.text, str, sizeof(msg.string.text) - 1);
	msg.string.text[sizeof(msg.string.text) - 1] = '\0';
	xQueueSend(display_queue, &msg, 0);
}

void display_va_string(int16_t x, int16_t y, const ssd130x_font_t *font, const char *fmt, ...)
{
	display_msg_t msg = { .type = DISPLAY_STRING };
	msg.string.x    = x;
	msg.string.y    = y;
	msg.string.font = font;
	va_list args;
	va_start(args, fmt);
	vsnprintf(msg.string.text, sizeof(msg.string.text), fmt, args);
	va_end(args);
	xQueueSend(display_queue, &msg, 0);
}

void display_flush(void)
{
	display_msg_t msg = { .type = DISPLAY_FLUSH };
	xQueueSend(display_queue, &msg, 0);
}
