/*
	SlimeVR-IMU Library
	Extracted from the SlimeVR firmware for use as a standalone library.

	SlimeVR Code is placed under the MIT license
	Copyright (c) 2021 SlimeVR Contributors

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

/**
 * @file SlimeVRIMU.h
 * @brief Main include header for the SlimeVR-IMU library.
 *
 * Include this single header to get access to SlimeVR's sensor fusion,
 * calibration, and IMU driver infrastructure.
 *
 * ## Quick Start
 *
 * ```cpp
 * #include <SlimeVRIMU.h>
 *
 * SlimeVR::Configuration::Configuration configuration;
 * SlimeVR::Sensors::SensorManager sensorManager;
 *
 * void setup() {
 *     Serial.begin(115200);
 *     sensorManager.setup();
 *     sensorManager.postSetup();
 * }
 *
 * void loop() {
 *     sensorManager.update();
 *     for (auto& sensor : sensorManager.getSensors()) {
 *         if (sensor->isWorking()) {
 *             Quat rotation = sensor->getFusedRotation();
 *             Vector3 accel = sensor->getAcceleration();
 *             // Use rotation and acceleration data...
 *         }
 *     }
 * }
 * ```
 *
 * ## Supported IMUs
 *
 * ### Software Fusion (VQF algorithm):
 * - BMI160, BMI270
 * - ICM42688, ICM45605, ICM45686
 * - LSM6DS3TRC, LSM6DSO, LSM6DSR, LSM6DSV
 * - MPU6050 (software fusion variant)
 *
 * ### Hardware DMP Fusion:
 * - BNO080, BNO085, BNO086, BNO055
 * - MPU6050, MPU9250
 * - ICM20948
 *
 * ## Configuration
 *
 * Set IMU type and pins via build flags in platformio.ini:
 * ```ini
 * build_flags =
 *   -DIMU=IMU_BMI270
 *   -DPIN_IMU_SDA=21
 *   -DPIN_IMU_SCL=22
 * ```
 *
 * Or define them before including this header:
 * ```cpp
 * #define IMU IMU_BMI270
 * #define PIN_IMU_SDA 21
 * #define PIN_IMU_SCL 22
 * #include <SlimeVRIMU.h>
 * ```
 */

#pragma once

// Core types
#include <quat.h>
#include <vector3.h>

// Constants and configuration
#include "consts.h"
#include "globals.h"

// Sensor infrastructure
#include "sensors/sensor.h"
#include "sensors/SensorManager.h"
#include "sensors/SensorFusion.h"

// Configuration and calibration
#include "configuration/Configuration.h"

// Logging
#include "logging/Logger.h"
