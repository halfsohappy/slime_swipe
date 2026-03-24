/**
 * Sensor Fusion Example
 *
 * Demonstrates how to use the SlimeVR-IMU library's VQF-based sensor
 * fusion algorithm directly for custom IMU data processing. This example
 * shows both the high-level SensorManager approach and the lower-level
 * SensorFusion API for more advanced use cases.
 *
 * The VQF (Versatile Quaternion-based Filter) algorithm provides:
 *   - 6-DOF fusion (accelerometer + gyroscope)
 *   - 9-DOF fusion (accelerometer + gyroscope + magnetometer)
 *   - Automatic rest detection and bias estimation
 *   - Configurable filter parameters
 *
 * Wiring:
 *   Connect your IMU's SDA/SCL pins to the ESP's I2C pins.
 *
 * Build Flags (platformio.ini):
 *   build_flags =
 *     -O2
 *     -std=gnu++2a
 */

#include <SlimeVRIMU.h>

// Required global instances
SlimeVR::Configuration::Configuration configuration;
SlimeVR::Sensors::SensorManager sensorManager;

// Tracking state
unsigned long lastPrintTime = 0;
constexpr unsigned long printInterval = 100;  // Print at 10Hz

void printQuaternionAsEuler(const Quat& q) {
	// Convert quaternion to Euler angles (roll, pitch, yaw) in degrees
	float sinr_cosp = 2.0f * (q.w * q.x + q.y * q.z);
	float cosr_cosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
	float roll = atan2(sinr_cosp, cosr_cosp) * 180.0f / PI;

	float sinp = 2.0f * (q.w * q.y - q.z * q.x);
	float pitch;
	if (fabs(sinp) >= 1.0f) {
		pitch = copysign(90.0f, sinp);
	} else {
		pitch = asin(sinp) * 180.0f / PI;
	}

	float siny_cosp = 2.0f * (q.w * q.z + q.x * q.y);
	float cosy_cosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
	float yaw = atan2(siny_cosp, cosy_cosp) * 180.0f / PI;

	Serial.printf("Roll: %7.2f° Pitch: %7.2f° Yaw: %7.2f°", roll, pitch, yaw);
}

void setup() {
	Serial.begin(115200);
	while (!Serial) {
		delay(10);
	}
	delay(1000);

	Serial.println("SlimeVR-IMU Library - Sensor Fusion Example");
	Serial.println("============================================");

	// Initialize configuration (for calibration data persistence)
	configuration.setup();

	// Initialize sensor manager (auto-detects and configures sensors)
	sensorManager.setup();
	sensorManager.postSetup();

	auto& sensors = sensorManager.getSensors();
	int workingCount = 0;
	for (auto& s : sensors) {
		if (s->isWorking()) {
			workingCount++;
		}
	}

	Serial.printf("Initialized %d working sensor(s)\n\n", workingCount);

	if (workingCount == 0) {
		Serial.println("ERROR: No working sensors found!");
		Serial.println("Check your wiring and IMU configuration.");
		Serial.println("Set -DIMU=IMU_YOUR_TYPE in build flags.");
	}
}

void loop() {
	// Process sensor data (reads hardware, runs fusion algorithm)
	sensorManager.update();

	unsigned long now = millis();
	if (now - lastPrintTime < printInterval) {
		return;
	}
	lastPrintTime = now;

	auto& sensors = sensorManager.getSensors();
	for (size_t i = 0; i < sensors.size(); i++) {
		auto& sensor = sensors[i];
		if (!sensor->isWorking()) {
			continue;
		}

		const Quat& q = sensor->getFusedRotation();
		const Vector3& a = sensor->getAcceleration();

		Serial.printf("[%s #%d] ", getIMUNameByType(sensor->getSensorType()), i);
		printQuaternionAsEuler(q);
		Serial.printf(" | Accel: %.2f %.2f %.2f m/s²", a.x, a.y, a.z);

		if (sensor->hasCompletedRestCalibration()) {
			Serial.print(" [calibrated]");
		}

		Serial.println();
	}
}
