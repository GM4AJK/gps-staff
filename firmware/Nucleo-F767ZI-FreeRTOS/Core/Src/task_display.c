#include "task_display.h"

#include <stdio.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "main.h"

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
			const ssd1309_font_t *font;
			char text[DISPLAY_TEXT_LEN];
		} string;
	};
} display_msg_t;

static QueueHandle_t display_queue;
static ssd1309_t oled;

static void display_task(void *arg)
{
	(void)arg;

	ssd1309_init(&oled, &hi2c1, 0x3C, -1, -1);
	xSemaphoreTake(hi2c1_mutex, portMAX_DELAY);
	ssd1309_bringup(&oled);
	xSemaphoreGive(hi2c1_mutex);

	display_msg_t msg;
	for (;;) {
		if (xQueueReceive(display_queue, &msg, portMAX_DELAY) != pdTRUE)
			continue;

		switch (msg.type) {
		case DISPLAY_CLEAR:
			ssd1309_clear(&oled);
			break;
		case DISPLAY_STRING:
			ssd1309_draw_string(&oled, msg.string.font,
				msg.string.x, msg.string.y,
				msg.string.text, SSD1309_COLOR_ON);
			break;
		case DISPLAY_FLUSH:
			xSemaphoreTake(hi2c1_mutex, portMAX_DELAY);
			ssd1309_flush(&oled);
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

void display_string(int16_t x, int16_t y, const ssd1309_font_t *font, const char *fmt, ...)
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
