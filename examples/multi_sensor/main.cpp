/**
 * SlimeIMU Multi-Sensor Example
 *
 * Demonstrates using multiple IMU sensors on the same I2C bus.
 * Most IMUs have two possible I2C addresses (e.g., 0x68 and 0x69)
 * selectable via an address pin (SDO/AD0).
 *
 * The library automatically detects all sensors present on the bus.
 *
 * Wiring for 2 sensors (example with BMI270):
 *   Sensor 1: SDO -> GND (address 0x68)
 *   Sensor 2: SDO -> VCC (address 0x69)
 *   Both share SDA, SCL, VCC, GND
 */

#include <Arduino.h>
#include <SlimeIMU.h>

#define SDA_PIN 21
#define SCL_PIN 22

SlimeIMU imu;

// Euler angles from quaternion (for human-readable output)
struct EulerAngles {
	float roll, pitch, yaw;
};

EulerAngles quatToEuler(const Quat& q) {
	EulerAngles e;

	// Roll (x-axis rotation)
	float sinr_cosp = 2.0f * (q.w * q.x + q.y * q.z);
	float cosr_cosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
	e.roll = atan2f(sinr_cosp, cosr_cosp) * 180.0f / PI;

	// Pitch (y-axis rotation)
	float sinp = 2.0f * (q.w * q.y - q.z * q.x);
	if (fabsf(sinp) >= 1.0f) {
		e.pitch = copysignf(90.0f, sinp);
	} else {
		e.pitch = asinf(sinp) * 180.0f / PI;
	}

	// Yaw (z-axis rotation)
	float siny_cosp = 2.0f * (q.w * q.z + q.x * q.y);
	float cosy_cosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
	e.yaw = atan2f(siny_cosp, cosy_cosp) * 180.0f / PI;

	return e;
}

void setup() {
	Serial.begin(115200);
	delay(1000);
	Serial.println("SlimeIMU Multi-Sensor Example");
	Serial.println("=============================");

	if (!imu.begin(SDA_PIN, SCL_PIN)) {
		Serial.println("ERROR: No IMU sensors detected!");
		while (true) delay(1000);
	}

	size_t count = imu.getSensorCount();
	Serial.printf("Detected %d sensor(s):\n", count);
	for (size_t i = 0; i < count; i++) {
		Serial.printf(
			"  [%d] %s - %s\n",
			i,
			imu.getSensorName(i),
			imu.isSensorWorking(i) ? "OK" : "ERROR"
		);
	}
	Serial.println();
}

void loop() {
	imu.update();

	static unsigned long lastPrint = 0;
	if (millis() - lastPrint >= 100) {
		lastPrint = millis();

		for (size_t i = 0; i < imu.getSensorCount(); i++) {
			if (!imu.isSensorWorking(i)) continue;

			Quat q = imu.getQuaternion(i);
			EulerAngles euler = quatToEuler(q);

			Serial.printf(
				"[%d %s] Roll: %6.1f° Pitch: %6.1f° Yaw: %6.1f°\n",
				i,
				imu.getSensorName(i),
				euler.roll,
				euler.pitch,
				euler.yaw
			);
		}
		if (imu.getSensorCount() > 0) Serial.println();
	}
}
