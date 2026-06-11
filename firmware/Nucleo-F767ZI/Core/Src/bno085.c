/*
 * bno085.c
 *
 *  Created on: 11 Jun 2026
 *      Author: kirkh
 */

#include <stdio.h>
#include <string.h>

#include "main.h"

#include "sh2.h"
#include "sh2_util.h"
#include "euler.h"
#include "sh2_err.h"
#include "sh2_SensorValue.h"
#include "sh2_hal_init.h"

static sh2_ProductIds_t prodIds;
static sh2_Hal_t *pSh2Hal = 0;
static bool resetOccurred = false;

static void eventHandler(void * cookie, sh2_AsyncEvent_t *pEvent)
{
	// If we see a reset, set a flag so that sensors will be reconfigured.
	if (pEvent->eventId == SH2_RESET) {
		resetOccurred = true;
	}
	else if (pEvent->eventId == SH2_SHTP_EVENT) {
		uint8_t buf[128];
		int len = snprintf((char*)buf, 128, "EventHandler  id:SHTP, %d\r\n", (int)pEvent->shtpEvent);
		HAL_UART_Transmit(&huart3, buf, len, 100);
	}
	else if (pEvent->eventId == SH2_GET_FEATURE_RESP) {
		uint8_t buf[128];
		int len = snprintf((char*)buf, 128, "EventHandler Sensor Config, %d\r\n", (int)pEvent->sh2SensorConfigResp.sensorId);
		HAL_UART_Transmit(&huart3, buf, len, 100);
		//printf("EventHandler Sensor Config, %d\r\n", (int)pEvent->sh2SensorConfigResp.sensorId);
	}
	else {
		uint8_t buf[128];
		int len = snprintf((char*)buf, 128, "EventHandler, unknown event Id: %d\r\n", (int)pEvent->eventId);
		HAL_UART_Transmit(&huart3, buf, len, 100);
    }
}

static void reportProdIds(void)
{
	int status;
	memset(&prodIds, 0, sizeof(prodIds));
	status = sh2_getProdIds(&prodIds);
	if (status < 0) {
		printf("Error from sh2_getProdIds.\r\n");
		return;
	}

	// Report the results
	for (int n = 0; n < prodIds.numEntries; n++) {
		uint8_t buf[128];
		int len = snprintf((char*)buf, 128, "Part %d : Version %d.%d.%d Build %d\r\n",
			(int)prodIds.entry[n].swPartNumber,
			(int)prodIds.entry[n].swVersionMajor, (int)prodIds.entry[n].swVersionMinor,
			(int)prodIds.entry[n].swVersionPatch, (int)prodIds.entry[n].swBuildNumber);
		HAL_UART_Transmit(&huart3, buf, len, 100);
		// Wait a bit so we don't overflow the console output.
		//delayUs(10000);
		HAL_Delay(10);
    }
}

static void printEvent(const sh2_SensorEvent_t * event)
{
    int rc;
    sh2_SensorValue_t value;
    float scaleRadToDeg = 180.0 / 3.14159265358;
    float r, i, j, k, acc_deg, x, y, z;
    float t;
    static int skip = 0;

    rc = sh2_decodeSensorEvent(&value, event);
    if (rc != SH2_OK) {
        printf("Error decoding sensor event: %d\r\n", rc);
        return;
    }

    t = value.timestamp / 1000000.0;  // time in seconds.
    switch (value.sensorId) {
        case SH2_RAW_ACCELEROMETER:
            printf("%8.4f Raw acc: %d %d %d time_us:%d\r\n",
                   (float)t,
				   (int)value.un.rawAccelerometer.x,
				   (int)value.un.rawAccelerometer.y,
				   (int)value.un.rawAccelerometer.z,
				   (int)value.un.rawAccelerometer.timestamp);
            break;

        case SH2_ACCELEROMETER:
            printf("%8.4f Acc: %f %f %f\r\n",
                   (float)t,
                   (float)value.un.accelerometer.x,
                   (float)value.un.accelerometer.y,
                   (float)value.un.accelerometer.z);
            break;

        case SH2_RAW_GYROSCOPE:
            printf("%8.4f Raw gyro: x:%d y:%d z:%d temp:%d time_us:%d\r\n",
                   (float)t,
                   (int)value.un.rawGyroscope.x,
				   (int)value.un.rawGyroscope.y,
				   (int)value.un.rawGyroscope.z,
				   (int)value.un.rawGyroscope.temperature,
				   (int)value.un.rawGyroscope.timestamp);
            break;

        case SH2_ROTATION_VECTOR:
            r = value.un.rotationVector.real;
            i = value.un.rotationVector.i;
            j = value.un.rotationVector.j;
            k = value.un.rotationVector.k;
            acc_deg = scaleRadToDeg *
                value.un.rotationVector.accuracy;
            printf("%8.4f Rotation Vector: "
                   "r:%0.6f i:%0.6f j:%0.6f k:%0.6f (acc: %0.6f deg)\r\n",
                   (float)t,
                   (float)r, (float)i, (float)j, (float)k, (float)acc_deg);
            break;
        case SH2_GAME_ROTATION_VECTOR:
            r = value.un.gameRotationVector.real;
            i = value.un.gameRotationVector.i;
            j = value.un.gameRotationVector.j;
            k = value.un.gameRotationVector.k;
            printf("%8.4f GRV: "
                   "r:%0.6f i:%0.6f j:%0.6f k:%0.6f\r\n",
                   (float)t,
                   (float)r, (float)i, (float)j, (float)k);
            break;
        case SH2_GYROSCOPE_CALIBRATED:
            x = value.un.gyroscope.x;
            y = value.un.gyroscope.y;
            z = value.un.gyroscope.z;
            printf("%8.4f GYRO: "
                   "x:%0.6f y:%0.6f z:%0.6f\r\n",
                   (float)t,
                   (float)x, (float)y, (float)z);
            break;
        case SH2_GYROSCOPE_UNCALIBRATED:
            x = value.un.gyroscopeUncal.x;
            y = value.un.gyroscopeUncal.y;
            z = value.un.gyroscopeUncal.z;
            printf("%8.4f GYRO_UNCAL: "
                   "x:%0.6f y:%0.6f z:%0.6f\r\n",
                   (float)t,
                   (float)x, (float)y, (float)z);
            break;
        case SH2_GYRO_INTEGRATED_RV:
            // These come at 1kHz, too fast to print all of them.
            // So only print every 10th one
            skip++;
            if (skip == 10) {
                skip = 0;
                r = value.un.gyroIntegratedRV.real;
                i = value.un.gyroIntegratedRV.i;
                j = value.un.gyroIntegratedRV.j;
                k = value.un.gyroIntegratedRV.k;
                x = value.un.gyroIntegratedRV.angVelX;
                y = value.un.gyroIntegratedRV.angVelY;
                z = value.un.gyroIntegratedRV.angVelZ;
                printf("%8.4f Gyro Integrated RV: "
                       "r:%0.6f i:%0.6f j:%0.6f k:%0.6f x:%0.6f y:%0.6f z:%0.6f\r\n",
                       (float)t,
                       (float)r, (float)i, (float)j, (float)k,
                       (float)x, (float)y, (float)z);
            }
            break;
        case SH2_IZRO_MOTION_REQUEST:
            printf("IZRO Request: intent:%d, request:%d\r\n",
                   value.un.izroRequest.intent,
                   value.un.izroRequest.request);
            break;
        case SH2_SHAKE_DETECTOR:
            printf("Shake Axis: %c%c%c\r\n",
                   (value.un.shakeDetector.shake & SHAKE_X) ? 'X' : '.',
                   (value.un.shakeDetector.shake & SHAKE_Y) ? 'Y' : '.',
                   (value.un.shakeDetector.shake & SHAKE_Z) ? 'Z' : '.');

            break;
        case SH2_STABILITY_CLASSIFIER:
            printf("Stability Classification: %d\r\n",
                   value.un.stabilityClassifier.classification);
            break;
        case SH2_STABILITY_DETECTOR:
            printf("Stability Detector: %d\r\n",
                   value.un.stabilityDetector.stability);
            break;
        default:
            printf("Unknown sensor: %d\r\n", value.sensorId);
            break;
    }
}

static void sensorHandler(void * cookie, sh2_SensorEvent_t *pEvent)
{
#ifdef DSF_OUTPUT
	printDsf(pEvent);
#else
	printEvent(pEvent);
#endif
}

static void startReports()
{
    int status;

    // Each entry of sensorConfig[] represents one sensor to be configured in the loop below
    static const struct {
        int sensorId;
        sh2_SensorConfig_t config;
    } sensorConfig[] =
    {
        // Game Rotation Vector, 100Hz
        {SH2_GAME_ROTATION_VECTOR, {.reportInterval_us = 10000}},

        // Stability Detector, 100 Hz, changeSensitivityEnabled
        // {SH2_STABILITY_DETECTOR, {.reportInterval_us = 10000, .changeSensitivityEnabled = true}},

        // Raw accel, 100 Hz
        // {SH2_RAW_ACCELEROMETER, {.reportInterval_us = 10000}},

        // Raw gyroscope, 100 Hz
        // {SH2_RAW_GYROSCOPE, {.reportInterval_us = 10000}},

        // Rotation Vector, 100 Hz
        // {SH2_ROTATION_VECTOR, {.reportInterval_us = 10000}},

        // Gyro Integrated Rotation Vector, 100 Hz
        // {SH2_GYRO_INTEGRATED_RV, {.reportInterval_us = 10000}},

        // Motion requests for Interactive Zero Reference Offset cal
        // {SH2_IZRO_MOTION_REQUEST, {.reportInterval_us = 10000}},

        // Shake detector
        // {SH2_SHAKE_DETECTOR, {.reportInterval_us = 10000}},
    };

    for (int n = 0; n < ARRAY_LEN(sensorConfig); n++)
    {
        int sensorId = sensorConfig[n].sensorId;

        status = sh2_setSensorConfig(sensorId, &sensorConfig[n].config);
        if (status != 0) {
            printf("Error while enabling sensor %d\r\n", sensorId);
        }
    }
}

void bno085_init(void)
{
	int status;
	pSh2Hal = sh2_hal_init();
	status = sh2_open(pSh2Hal, eventHandler, NULL);
	if (status != SH2_OK) {
		uint8_t buf[128];
		int len = snprintf((char*)buf, 128, "Error, %d, from sh2_open.\r\n", status);
		HAL_UART_Transmit(&huart3, buf, len, 100);
	}
	sh2_setSensorCallback(sensorHandler, NULL);
	reportProdIds();
	resetOccurred = false;
	startReports();
}

void bno085_service(void)
{
	uint32_t now = pSh2Hal->getTimeUs(pSh2Hal);
	if (resetOccurred) {
		// Restart the flow of sensor reports
		resetOccurred = false;
		startReports();
	}

	// Service the sensor hub.
	// Sensor reports and event processing handled by callbacks.
	sh2_service();
}

