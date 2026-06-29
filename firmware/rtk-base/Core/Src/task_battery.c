#include "task_battery.h"
#include "main.h"
#include "cmsis_os.h"

/*
 * Resistor divider: R1 = R2 = 100k (VBAT → PA1 → GND).
 * V_PA1 = V_BAT / 2  →  V_BAT = V_PA1 × 2.
 *
 * ADC1 CH1, 12-bit, VDDA = 3300 mV (software-triggered single conversion,
 * configured by CubeMX MX_ADC1_Init).
 *
 * Full conversion:
 *   vbat_mv = adc_raw × 3300 × 2 / 4095
 *           = adc_raw × 6600 / 4095
 *
 * Percentage (LiPo 3.0–4.2 V, span 1200 mV):
 *   pct = (vbat_mv − 3000) × 100 / 1200
 */

#define BATT_SAMPLE_PERIOD_MS	10000U
#define BATT_NUM_SAMPLES	4U
#define BATT_VDDA_MV		3300UL
#define BATT_ADC_MAX		4095UL
#define BATT_DIVIDER_MULT	2UL
#define BATT_EMPTY_MV		3000UL
#define BATT_FULL_MV		4200UL
#define BATT_SPAN_MV		(BATT_FULL_MV - BATT_EMPTY_MV)

extern ADC_HandleTypeDef hadc1;

volatile uint8_t  g_battery_pct = 0;
volatile uint16_t g_battery_mv  = 0;

static uint32_t adc_sample_raw(void)
{
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 10);
	uint32_t raw = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	return raw;
}

void task_battery(void *argument)
{
	(void)argument;

	for (;;)
	{
		uint32_t acc = 0;
		for (uint32_t i = 0; i < BATT_NUM_SAMPLES; i++)
			acc += adc_sample_raw();

		uint32_t vbat_mv = (acc / BATT_NUM_SAMPLES) * BATT_VDDA_MV * BATT_DIVIDER_MULT / BATT_ADC_MAX;

		if (vbat_mv < BATT_EMPTY_MV) vbat_mv = BATT_EMPTY_MV;
		if (vbat_mv > BATT_FULL_MV)  vbat_mv = BATT_FULL_MV;

		g_battery_mv  = (uint16_t)vbat_mv;
		g_battery_pct = (uint8_t)((vbat_mv - BATT_EMPTY_MV) * 100UL / BATT_SPAN_MV);

		osDelay(BATT_SAMPLE_PERIOD_MS);
	}
}
