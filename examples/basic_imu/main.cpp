/**
 * SlimeIMU Basic Example
 *
 * Demonstrates basic IMU reading with automatic sensor detection.
 * Supports all SlimeVR-compatible IMUs (BMI270, LSM6DSV, ICM42688, etc.)
 *
 * Wiring:
 *   IMU SDA -> ESP32 GPIO 21 (or your chosen SDA pin)
 *   IMU SCL -> ESP32 GPIO 22 (or your chosen SCL pin)
 *   IMU VCC -> 3.3V
 *   IMU GND -> GND
 */

#include <Arduino.h>
#include <SlimeIMU.h>

// Adjust these to match your wiring
#define SDA_PIN 21
#define SCL_PIN 22

SlimeIMU imu;

void setup() {
	Serial.begin(115200);
	delay(1000);
	Serial.println("SlimeIMU Basic Example");
	Serial.println("======================");

	if (!imu.begin(SDA_PIN, SCL_PIN)) {
		Serial.println("ERROR: No IMU sensors detected!");
		Serial.println("Check your wiring and I2C address.");
		while (true) delay(1000);
	}

	Serial.printf("Detected %d sensor(s)\n", imu.getSensorCount());
	for (size_t i = 0; i < imu.getSensorCount(); i++) {
		if (imu.isSensorWorking(i)) {
			Serial.printf("  Sensor %d: %s\n", i, imu.getSensorName(i));
		}
	}

	Serial.println("\nReading orientation data...\n");
}

void loop() {
	imu.update();

	// Print quaternion data at ~10Hz
	static unsigned long lastPrint = 0;
	if (millis() - lastPrint >= 100) {
		lastPrint = millis();

		for (size_t i = 0; i < imu.getSensorCount(); i++) {
			if (!imu.isSensorWorking(i)) continue;

			Quat q = imu.getQuaternion(i);
			Vector3 accel = imu.getLinearAcceleration(i);

			Serial.printf(
				"[Sensor %d] Quat: w=%.3f x=%.3f y=%.3f z=%.3f  "
				"Accel: x=%.2f y=%.2f z=%.2f m/s²\n",
				i,
				q.w,
				q.x,
				q.y,
				q.z,
				accel.x,
				accel.y,
				accel.z
			);
		}
	}
}
