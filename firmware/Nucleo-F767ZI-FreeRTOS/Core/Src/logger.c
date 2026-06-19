#include "logger.h"

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "main.h"

#define LOG_MSG_LEN   128
#define LOG_QUEUE_LEN   16

static QueueHandle_t log_queue;

static void logger_task(void *arg)
{
    (void)arg;
    char msg[LOG_MSG_LEN];

    for (;;) {
        if (xQueueReceive(log_queue, msg, portMAX_DELAY) == pdTRUE)
            HAL_UART_Transmit(&huart3, (uint8_t *)msg, strlen(msg), 100);
    }
}

void logger_init(void)
{
    log_queue = xQueueCreate(LOG_QUEUE_LEN, LOG_MSG_LEN);
    configASSERT(log_queue);
    xTaskCreate(logger_task, "logger", 256, NULL, tskIDLE_PRIORITY + 2, NULL);
}

void logger_log(const char *fmt, ...)
{
    char msg[LOG_MSG_LEN];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);
    xQueueSend(log_queue, msg, 0);
}
