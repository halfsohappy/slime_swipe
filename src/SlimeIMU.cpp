/*
	SlimeIMU Library - Extracted from SlimeVR Firmware
	Based on SlimeVR Code, placed under the MIT license
	Copyright (c) 2021-2025 Eiren Rain & SlimeVR contributors
*/

#include "SlimeIMU.h"

#include <i2cscan.h>
#include <SPI.h>

#include "GlobalVars.h"
#include "configuration/Configuration.h"
#include "defines.h"
#include "logging/Logger.h"

// Global instances required by the internal sensor/calibration code
Timer<> globalTimer;
SlimeVR::Logging::Logger logger("SlimeIMU");
SlimeVR::Sensors::SensorManager sensorManager;
SlimeVR::LEDManager ledManager;
SlimeVR::Status::StatusManager statusManager;
SlimeVR::Configuration::Configuration configuration;

bool SlimeIMU::begin(uint8_t sdaPin, uint8_t sclPin) {
	SlimeIMUConfig config;
	config.sdaPin = sdaPin;
	config.sclPin = sclPin;
	return begin(config);
}

bool SlimeIMU::begin(const SlimeIMUConfig& config) {
	if (m_initialized) {
		return true;
	}

	logger.info("SlimeIMU initializing...");

	globalTimer = timer_create_default();

	// Initialize calibration storage
	configuration.setup();

#if IMU_PROTOCOL == IMU_PROTOCOL_I2C
		// Clear I2C bus in case of stuck state
		auto clearResult = I2CSCAN::clearBus(config.sdaPin, config.sclPin);
		if (clearResult != 0) {
			logger.warn("Can't clear I2C bus, error %d", clearResult);
		}

#ifdef ESP32
		Wire.end();
#endif

		Wire.begin(static_cast<int>(config.sdaPin), static_cast<int>(config.sclPin));

#ifdef ESP8266
		Wire.setClockStretchLimit(150000L);
#endif
#ifdef ESP32
		Wire.setTimeOut(150);
#endif
		Wire.setClock(config.i2cSpeed);
#else
		uint8_t spiSckPin = config.spiSckPin == 255 ? PIN_IMU_SPI_SCK : config.spiSckPin;
		uint8_t spiMisoPin
			= config.spiMisoPin == 255 ? PIN_IMU_SPI_MISO : config.spiMisoPin;
		uint8_t spiMosiPin
			= config.spiMosiPin == 255 ? PIN_IMU_SPI_MOSI : config.spiMosiPin;
#ifdef ESP32
		if (spiSckPin != 255 && spiMisoPin != 255 && spiMosiPin != 255) {
			SPI.begin(spiSckPin, spiMisoPin, spiMosiPin);
		} else {
			SPI.begin();
		}
#else
		SPI.begin();
#endif
#endif

	// Wait for IMU to boot
	delay(500);

	// Detect and initialize all sensors
	sensorManager.setup();
	sensorManager.postSetup();

	auto& sensors = sensorManager.getSensors();
	size_t workingCount = 0;
	for (auto& sensor : sensors) {
		if (sensor->isWorking()) {
			workingCount++;
		}
	}

	logger.info("%d sensor(s) active", workingCount);
	m_initialized = true;
	return workingCount > 0;
}

void SlimeIMU::update() {
	if (!m_initialized) return;
	globalTimer.tick();
	sensorManager.update();
}

size_t SlimeIMU::getSensorCount() const {
	return sensorManager.getSensors().size();
}

bool SlimeIMU::isSensorWorking(size_t sensorIndex) const {
	auto& sensors = sensorManager.getSensors();
	if (sensorIndex >= sensors.size()) return false;
	return sensors[sensorIndex]->isWorking();
}

bool SlimeIMU::hasNewData(size_t sensorIndex) const {
	auto& sensors = sensorManager.getSensors();
	if (sensorIndex >= sensors.size()) return false;
	return sensors[sensorIndex]->hasNewDataToSend();
}

Quat SlimeIMU::getQuaternion(size_t sensorIndex) const {
	auto& sensors = sensorManager.getSensors();
	if (sensorIndex >= sensors.size()) return Quat();
	return sensors[sensorIndex]->getFusedRotation();
}

Vector3 SlimeIMU::getLinearAcceleration(size_t sensorIndex) const {
	auto& sensors = sensorManager.getSensors();
	if (sensorIndex >= sensors.size()) return Vector3();
	return sensors[sensorIndex]->getAcceleration();
}

SensorTypeID SlimeIMU::getSensorType(size_t sensorIndex) const {
	return sensorManager.getSensorType(sensorIndex);
}

const char* SlimeIMU::getSensorName(size_t sensorIndex) const {
	return getIMUNameByType(getSensorType(sensorIndex));
}

void SlimeIMU::startCalibration(size_t sensorIndex, int calibrationType) {
	auto& sensors = sensorManager.getSensors();
	if (sensorIndex >= sensors.size()) return;
	if (!sensors[sensorIndex]->isWorking()) return;
	sensors[sensorIndex]->startCalibration(calibrationType);
}

void SlimeIMU::saveCalibration() {
	configuration.save();
}

bool SlimeIMU::isRestCalibrationComplete(size_t sensorIndex) const {
	auto& sensors = sensorManager.getSensors();
	if (sensorIndex >= sensors.size()) return false;
	return sensors[sensorIndex]->hasCompletedRestCalibration();
}

SlimeVR::Sensors::SensorManager& SlimeIMU::getSensorManager() {
	return sensorManager;
}
