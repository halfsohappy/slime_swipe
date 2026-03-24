/*
	SlimeIMU Library - Extracted from SlimeVR Firmware
	Based on SlimeVR Code, placed under the MIT license
	Copyright (c) 2021-2025 Eiren Rain & SlimeVR contributors

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
 * SlimeIMU - High-quality IMU sensor fusion library for ESP32
 *
 * Extracted from SlimeVR's firmware, providing access to their excellent
 * sensor fusion (VQF), calibration routines, and broad IMU driver support.
 *
 * Supported IMUs (via software fusion):
 *   BMI160, BMI270, ICM42688, ICM45605, ICM45686,
 *   LSM6DS3TRC, LSM6DSO, LSM6DSR, LSM6DSV, MPU6050
 *
 * Supported IMUs (via hardware fusion):
 *   BNO055, BNO080, BNO085, BNO086, ICM20948
 *
 * Features:
 *   - VQF (Versatile Quaternion-based Filter) sensor fusion
 *   - Automatic IMU detection via WHOAMI registers
 *   - 6-axis and 9-axis (with magnetometer) fusion
 *   - Gyroscope offset calibration with temperature compensation
 *   - 6-point accelerometer calibration
 *   - Runtime continuous calibration option
 *   - Calibration persistence to flash (LittleFS)
 *   - I2C and SPI sensor interfaces
 *   - I2C multiplexer (PCA9547) and GPIO expander (MCP23X17) support
 *
 * Basic usage:
 *   #include <SlimeIMU.h>
 *
 *   SlimeIMU imu;
 *
 *   void setup() {
 *       Serial.begin(115200);
 *       imu.begin(SDA_PIN, SCL_PIN);
 *   }
 *
 *   void loop() {
 *       imu.update();
 *       if (imu.hasNewData(0)) {
 *           auto quat = imu.getQuaternion(0);
 *           auto accel = imu.getLinearAcceleration(0);
 *       }
 *   }
 */

#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <quat.h>
#include <vector3.h>

#include "consts.h"
#include "sensors/SensorManager.h"

struct SlimeIMUConfig {
	uint8_t sdaPin = 21;
	uint8_t sclPin = 22;
	uint32_t i2cSpeed = 400000;
	uint8_t spiSckPin = 255;
	uint8_t spiMisoPin = 255;
	uint8_t spiMosiPin = 255;
	bool useRuntimeCalibration = true;
};

class SlimeIMU {
public:
	SlimeIMU() = default;

	/**
	 * Initialize the IMU library with default I2C pins.
	 * Auto-detects connected IMU sensors.
	 * @param sdaPin I2C SDA pin number
	 * @param sclPin I2C SCL pin number
	 * @return true if at least one sensor was found
	 */
	bool begin(uint8_t sdaPin = 21, uint8_t sclPin = 22);

	/**
	 * Initialize with full configuration.
	 */
	bool begin(const SlimeIMUConfig& config);

	/**
	 * Call this in your loop() to read and process sensor data.
	 * Reads FIFO buffers, runs sensor fusion, updates quaternions.
	 */
	void update();

	/**
	 * Get the number of detected sensors.
	 */
	size_t getSensorCount() const;

	/**
	 * Check if a sensor is working.
	 */
	bool isSensorWorking(size_t sensorIndex) const;

	/**
	 * Check if sensor has new fused data since last read.
	 */
	bool hasNewData(size_t sensorIndex) const;

	/**
	 * Get the fused orientation quaternion for a sensor.
	 * Returns identity quaternion if sensor not available.
	 * Quaternion format: (x, y, z, w)
	 */
	Quat getQuaternion(size_t sensorIndex) const;

	/**
	 * Get linear acceleration (gravity removed) for a sensor.
	 * Returns zero vector if sensor not available.
	 * Units: m/s²
	 */
	Vector3 getLinearAcceleration(size_t sensorIndex) const;

	/**
	 * Get the detected sensor type for a given index.
	 */
	SensorTypeID getSensorType(size_t sensorIndex) const;

	/**
	 * Get a human-readable name for the detected sensor.
	 */
	const char* getSensorName(size_t sensorIndex) const;

	/**
	 * Trigger calibration for a specific sensor.
	 * @param sensorIndex Which sensor to calibrate
	 * @param calibrationType 0=ALL, 1=sample rate, 2=gyro offset, 3=accel, 4=motionless
	 */
	void startCalibration(size_t sensorIndex, int calibrationType = 0);

	/**
	 * Save current calibration data to flash.
	 */
	void saveCalibration();

	/**
	 * Check if initial rest calibration is complete for a sensor.
	 */
	bool isRestCalibrationComplete(size_t sensorIndex) const;

	/**
	 * Direct access to the underlying sensor manager for advanced use.
	 */
	SlimeVR::Sensors::SensorManager& getSensorManager();

private:
	bool m_initialized = false;
};
