/*
	SlimeVR Code is placed under the MIT license
	Copyright (c) 2022 TheDevMinerTV

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

#include "Configuration.h"

#include <Preferences.h>
#include <cstdint>
#include <cstring>

#include "consts.h"
#include "sensors/SensorToggles.h"
#include "utils.h"

// NVS namespace (≤15 chars)
#define PREFS_NS     "slimeimu"
#define KEY_CFG_VER  "cfg_ver"
#define KEY_NSENSORS "nsensors"

// Build a per-sensor NVS key.  Sensor index fits in uint8_t so the longest
// possible key is e.g. "scfg_255" = 8 chars, well within the 15-char limit.
static void sensorKey(char* buf, const char* prefix, uint8_t id) {
	snprintf(buf, 16, "%s_%u", prefix, id);
}

namespace SlimeVR::Configuration {

static Preferences prefs;

void Configuration::setup() {
	if (m_Loaded) {
		return;
	}

	prefs.begin(PREFS_NS, false);

	int32_t storedVersion = prefs.getInt(KEY_CFG_VER, -1);

	if (storedVersion < 0) {
		m_Logger.info("No configuration file found, creating new one");
		m_Config.version = CURRENT_CONFIGURATION_VERSION;
		save();
	} else {
		m_Config.version = storedVersion;

		if (m_Config.version < CURRENT_CONFIGURATION_VERSION) {
			m_Logger.debug(
				"Configuration is outdated: v%d < v%d",
				m_Config.version,
				CURRENT_CONFIGURATION_VERSION
			);

			if (!runMigrations(m_Config.version)) {
				m_Logger.error(
					"Failed to migrate configuration from v%d to v%d",
					m_Config.version,
					CURRENT_CONFIGURATION_VERSION
				);
				return;
			}
		} else {
			m_Logger.info("Found up-to-date configuration v%d", m_Config.version);
		}
	}

	loadSensors();

	m_Loaded = true;

	m_Logger.info("Loaded configuration");

#ifdef DEBUG_CONFIGURATION
	print();
#endif
}

void Configuration::save() {
	prefs.putInt(KEY_CFG_VER, m_Config.version);

	uint32_t n = (uint32_t)m_Sensors.size();
	prefs.putUInt(KEY_NSENSORS, n);

	char key[16];
	for (size_t i = 0; i < n; i++) {
		SensorConfig config = m_Sensors[i];
		if (config.type == SensorConfigType::NONE) {
			continue;
		}

		m_Logger.trace("Saving sensor config data for %d", i);
		sensorKey(key, "scfg", (uint8_t)i);
		prefs.putBytes(key, &config, sizeof(SensorConfig));

		if (i < m_SensorToggles.size()) {
			m_Logger.trace("Saving sensor toggle state for %d", i);
			sensorKey(key, "stog", (uint8_t)i);
			auto toggleValues = m_SensorToggles[i].getValues();
			prefs.putBytes(key, &toggleValues, sizeof(SensorToggleValues));
		} else {
			m_Logger.trace(
				"Skipping saving toggles for sensor %d, no toggles present",
				i
			);
		}
	}

	m_Logger.debug("Saved configuration");
}

void Configuration::reset() {
	prefs.clear();

	m_Sensors.clear();
	m_SensorToggles.clear();
	m_Config.version = CURRENT_CONFIGURATION_VERSION;
	save();

	m_Logger.debug("Reset configuration");
}

int32_t Configuration::getVersion() const { return m_Config.version; }

size_t Configuration::getSensorCount() const { return m_Sensors.size(); }

SensorConfig Configuration::getSensor(size_t sensorID) const {
	if (sensorID >= m_Sensors.size()) {
		return {};
	}

	return m_Sensors.at(sensorID);
}

void Configuration::setSensor(size_t sensorID, const SensorConfig& config) {
	if (sensorID >= m_Sensors.size()) {
		m_Sensors.resize(sensorID + 1);
	}

	m_Sensors[sensorID] = config;
}

SensorToggleState Configuration::getSensorToggles(size_t sensorId) const {
	if (sensorId >= m_SensorToggles.size()) {
		return {};
	}

	return m_SensorToggles.at(sensorId);
}

void Configuration::setSensorToggles(size_t sensorId, SensorToggleState state) {
	if (sensorId >= m_SensorToggles.size()) {
		m_SensorToggles.resize(sensorId + 1);
	}

	m_SensorToggles[sensorId] = state;
}

void Configuration::eraseSensors() {
	char key[16];
	uint32_t n = prefs.getUInt(KEY_NSENSORS, 0);
	for (uint32_t i = 0; i < n; i++) {
		sensorKey(key, "scfg", (uint8_t)i);
		prefs.remove(key);
		sensorKey(key, "stog", (uint8_t)i);
		prefs.remove(key);
	}

	m_Sensors.clear();
	m_SensorToggles.clear();
	save();
}

void Configuration::loadSensors() {
	char key[16];
	uint32_t n = prefs.getUInt(KEY_NSENSORS, 0);

	for (uint32_t i = 0; i < n; i++) {
		sensorKey(key, "scfg", (uint8_t)i);
		if (!prefs.isKey(key)) {
			continue;
		}

		if (prefs.getBytesLength(key) != sizeof(SensorConfig)) {
			continue;
		}

		SensorConfig sensorConfig;
		prefs.getBytes(key, &sensorConfig, sizeof(SensorConfig));

		m_Logger.debug(
			"Found sensor calibration for %s at index %d",
			calibrationConfigTypeToString(sensorConfig.type),
			i
		);

		if (sensorConfig.type == SensorConfigType::BNO0XX) {
			SensorToggleState toggles;
			toggles.setToggle(
				SensorToggles::MagEnabled,
				sensorConfig.data.bno0XX.magEnabled
			);
			setSensorToggles(i, toggles);
		}

		setSensor(i, sensorConfig);

		sensorKey(key, "stog", (uint8_t)i);
		if (!prefs.isKey(key)) {
			continue;
		}

		SensorToggleValues values{};
		prefs.getBytes(key, &values, sizeof(SensorToggleValues));
		m_Logger.debug("Found sensor toggle state at index %d", i);
		setSensorToggles(i, SensorToggleState{values});
	}
}

bool Configuration::loadTemperatureCalibration(
	uint8_t sensorId,
	GyroTemperatureCalibrationConfig& config
) {
	char key[16];
	sensorKey(key, "tcfg", sensorId);

	if (!prefs.isKey(key)) {
		return false;
	}

	if (prefs.getBytesLength(key) != sizeof(GyroTemperatureCalibrationConfig)) {
		m_Logger.debug(
			"Found incompatible sensor temperature calibration (size mismatch) "
			"sensorId:%d, skipping",
			sensorId
		);
		return false;
	}

	// GyroTemperatureCalibrationConfig has no default constructor, so use a raw
	// buffer to check type compatibility before overwriting caller's config.
	alignas(GyroTemperatureCalibrationConfig)
		uint8_t buf[sizeof(GyroTemperatureCalibrationConfig)];
	prefs.getBytes(key, buf, sizeof(buf));

	SensorConfigType storedType;
	memcpy(&storedType, buf, sizeof(storedType));

	if (storedType != config.type) {
		m_Logger.debug(
			"Found incompatible sensor temperature calibration (expected %s, "
			"found %s) sensorId:%d, skipping",
			calibrationConfigTypeToString(config.type),
			calibrationConfigTypeToString(storedType),
			sensorId
		);
		return false;
	}

	memcpy(&config, buf, sizeof(config));
	m_Logger.debug(
		"Found sensor temperature calibration for %s sensorId:%d",
		calibrationConfigTypeToString(config.type),
		sensorId
	);
	return true;
}

bool Configuration::saveTemperatureCalibration(
	uint8_t sensorId,
	const GyroTemperatureCalibrationConfig& config
) {
	if (config.type == SensorConfigType::NONE) {
		return false;
	}

	char key[16];
	sensorKey(key, "tcfg", sensorId);

	m_Logger.trace("Saving temperature calibration data for sensorId:%d", sensorId);
	prefs.putBytes(key, &config, sizeof(GyroTemperatureCalibrationConfig));
	m_Logger.debug("Saved temperature calibration data for sensorId:%i", sensorId);
	return true;
}

bool Configuration::runMigrations(int32_t version) { return true; }

void Configuration::print() {
	m_Logger.info("Configuration:");
	m_Logger.info("  Version: %d", m_Config.version);
	m_Logger.info("  %d Sensors:", m_Sensors.size());

	for (size_t i = 0; i < m_Sensors.size(); i++) {
		const SensorConfig& c = m_Sensors[i];
		m_Logger.info("    - [%3d] %s", i, calibrationConfigTypeToString(c.type));

		switch (c.type) {
			case SensorConfigType::NONE:
				break;

			case SensorConfigType::BMI160:
				m_Logger.info(
					"            A_B        : %f, %f, %f",
					UNPACK_VECTOR_ARRAY(c.data.bmi160.A_B)
				);

				m_Logger.info("            A_Ainv     :");
				for (uint8_t i = 0; i < 3; i++) {
					m_Logger.info(
						"                         %f, %f, %f",
						UNPACK_VECTOR_ARRAY(c.data.bmi160.A_Ainv[i])
					);
				}

				m_Logger.info(
					"            G_off      : %f, %f, %f",
					UNPACK_VECTOR_ARRAY(c.data.bmi160.G_off)
				);
				m_Logger.info("            Temperature: %f", c.data.bmi160.temperature);

				break;

			case SensorConfigType::SFUSION:
				m_Logger.info(
					"            A_B        : %f, %f, %f",
					UNPACK_VECTOR_ARRAY(c.data.sfusion.A_B)
				);

				m_Logger.info("            A_Ainv     :");
				for (uint8_t i = 0; i < 3; i++) {
					m_Logger.info(
						"                         %f, %f, %f",
						UNPACK_VECTOR_ARRAY(c.data.sfusion.A_Ainv[i])
					);
				}

				m_Logger.info(
					"            G_off      : %f, %f, %f",
					UNPACK_VECTOR_ARRAY(c.data.sfusion.G_off)
				);
				m_Logger.info(
					"            Temperature: %f",
					c.data.sfusion.temperature
				);
				break;

			case SensorConfigType::ICM20948:
				m_Logger.info(
					"            G: %d, %d, %d",
					UNPACK_VECTOR_ARRAY(c.data.icm20948.G)
				);
				m_Logger.info(
					"            A: %d, %d, %d",
					UNPACK_VECTOR_ARRAY(c.data.icm20948.A)
				);
				m_Logger.info(
					"            C: %d, %d, %d",
					UNPACK_VECTOR_ARRAY(c.data.icm20948.C)
				);

				break;

			case SensorConfigType::MPU9250:
				m_Logger.info(
					"            A_B   : %f, %f, %f",
					UNPACK_VECTOR_ARRAY(c.data.mpu9250.A_B)
				);

				m_Logger.info("            A_Ainv:");
				for (uint8_t i = 0; i < 3; i++) {
					m_Logger.info(
						"                    %f, %f, %f",
						UNPACK_VECTOR_ARRAY(c.data.mpu9250.A_Ainv[i])
					);
				}

				m_Logger.info(
					"            M_B   : %f, %f, %f",
					UNPACK_VECTOR_ARRAY(c.data.mpu9250.M_B)
				);

				m_Logger.info("            M_Ainv:");
				for (uint8_t i = 0; i < 3; i++) {
					m_Logger.info(
						"                    %f, %f, %f",
						UNPACK_VECTOR_ARRAY(c.data.mpu9250.M_Ainv[i])
					);
				}

				m_Logger.info(
					"            G_off  : %f, %f, %f",
					UNPACK_VECTOR_ARRAY(c.data.mpu9250.G_off)
				);

				break;

			case SensorConfigType::MPU6050:
				m_Logger.info(
					"            A_B  : %f, %f, %f",
					UNPACK_VECTOR_ARRAY(c.data.mpu6050.A_B)
				);
				m_Logger.info(
					"            G_off: %f, %f, %f",
					UNPACK_VECTOR_ARRAY(c.data.mpu6050.G_off)
				);

				break;

			case SensorConfigType::BNO0XX:
				m_Logger.info("            magEnabled: %d", c.data.bno0XX.magEnabled);

				break;
		}
	}
}
}  // namespace SlimeVR::Configuration
