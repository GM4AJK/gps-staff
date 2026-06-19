#pragma once

#include <stdarg.h>

void logger_init(void);
void logger_log(const char *fmt, ...);
