#include "task_display.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "main.h"

#define DISPLAY_QUEUE_LEN 8

static QueueHandle_t display_queue;
static ssd1309_t oled;

static void display_task(void *arg)
{
	(void)arg;

	ssd1309_init(&oled, &hi2c1, 0x3C, -1, -1);
	ssd1309_bringup(&oled);

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
			ssd1309_flush(&oled);
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

void display_send(const display_msg_t *msg)
{
	xQueueSend(display_queue, msg, 0);
}
