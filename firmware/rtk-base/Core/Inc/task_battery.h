#pragma once

#include <stdint.h>

/*
 * Battery voltage monitor — samples VBATT_IN (PA1, ADC1 CH1) via a 100k/100k
 * resistor divider (ratio 0.5).  Updated every 10 s from task_battery().
 *
 * g_battery_pct : 0–100 %.  Clamped to LiPo range 3.0–4.2 V.
 * g_battery_mv  : raw millivolt reading after divider correction.
 *
 * Both are volatile uint8/uint16 — ARMv7-M guarantees aligned sub-word
 * accesses are atomic, so no mutex is needed for single-reader/single-writer.
 */
extern volatile uint8_t  g_battery_pct;
extern volatile uint16_t g_battery_mv;

void task_battery(void *argument);
