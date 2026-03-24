/*
	SlimeVR Code is placed under the MIT license
	Copyright (c) 2021 Eiren Rain

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/
#ifndef SLIMEVR_DEBUG_H_
#define SLIMEVR_DEBUG_H_
#include "consts.h"
#include "logging/Level.h"

#define BNO_USE_ARVR_STABILIZATION true
#define USE_6_AXIS true
#define LOAD_BIAS true
#define SAVE_BIAS true
#define BIAS_DEBUG false
#define ENABLE_TAP false
#define SEND_ACCELERATION true

// Debug information
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

#if LOG_LEVEL == LOG_LEVEL_TRACE
#define DEBUG_SENSOR
#define DEBUG_CONFIGURATION
#endif

#ifndef serialBaudRate
#define serialBaudRate 115200
#endif

// Determines how often we sample and send data
#define samplingRateInMillis 10

// Packet bundling (kept for compatibility, unused without network)
#define PACKET_BUNDLING PACKET_BUNDLING_DISABLED

// Magnetometer calibration
#define useFullCalibrationMatrix true

// Send updates only when changes are substantial
#define OPTIMIZE_UPDATES true

#ifndef I2C_SPEED
#define I2C_SPEED 400000
#endif

// Inspection packets disabled (no network)
#define ENABLE_INSPECTION false

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "SlimeIMU"
#endif

#ifndef USE_RUNTIME_CALIBRATION
#define USE_RUNTIME_CALIBRATION true
#endif

#define DEBUG_MEASURE_SENSOR_TIME_TAKEN false

#endif  // SLIMEVR_DEBUG_H_
