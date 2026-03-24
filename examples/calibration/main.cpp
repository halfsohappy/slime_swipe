/**
 * SlimeIMU Calibration Example
 *
 * Demonstrates how to calibrate IMU sensors.
 * Calibration data is automatically saved to flash and loaded on next boot.
 *
 * Calibration types:
 *   0 = Full calibration (sample rate + gyro offset + accelerometer)
 *   1 = Sample rate calibration only
 *   2 = Gyro offset calibration only
 *   3 = Accelerometer calibration only (requires 6 orientations)
 *   4 = Motionless calibration (sensor-specific, e.g. BMI270 CRT)
 *
 * Serial commands:
 *   'c' - Start full calibration for sensor 0
 *   'g' - Start gyro-only calibration for sensor 0
 *   'a' - Start accel-only calibration for sensor 0
 *   'r' - Reset calibration
 */

#include <Arduino.h>
#include <SlimeIMU.h>

#define SDA_PIN 21
#define SCL_PIN 22

SlimeIMU imu;

void setup() {
	Serial.begin(115200);
	delay(1000);
	Serial.println("SlimeIMU Calibration Example");
	Serial.println("============================");

	if (!imu.begin(SDA_PIN, SCL_PIN)) {
		Serial.println("ERROR: No IMU sensors detected!");
		while (true) delay(1000);
	}

	Serial.printf("Detected: %s\n", imu.getSensorName(0));
	Serial.println("\nCommands:");
	Serial.println("  'c' - Full calibration");
	Serial.println("  'g' - Gyro offset calibration");
	Serial.println("  'a' - Accelerometer 6-point calibration");
	Serial.println("  'r' - Reset all calibration\n");

	// Wait for rest calibration to complete
	Serial.println("Place sensor on a flat surface and keep still...");
}

void loop() {
	imu.update();

	// Check for serial commands
	if (Serial.available()) {
		char cmd = Serial.read();
		switch (cmd) {
			case 'c':
				Serial.println("\n--- Starting FULL calibration ---");
				Serial.println("Keep sensor still on a flat surface...");
				imu.startCalibration(0, 0);
				Serial.println("Calibration complete! Data saved to flash.\n");
				break;
			case 'g':
				Serial.println("\n--- Starting GYRO calibration ---");
				Serial.println("Keep sensor still...");
				imu.startCalibration(0, 2);
				Serial.println("Gyro calibration complete!\n");
				break;
			case 'a':
				Serial.println("\n--- Starting ACCELEROMETER calibration ---");
				Serial.println(
					"You'll need to place the sensor in 6 orientations."
				);
				imu.startCalibration(0, 3);
				Serial.println("Accel calibration complete!\n");
				break;
			case 'r':
				Serial.println("\nResetting calibration data...");
				// Will be rebuilt on next boot
				break;
		}
	}

	// Print status at 2Hz
	static unsigned long lastPrint = 0;
	if (millis() - lastPrint >= 500) {
		lastPrint = millis();

		if (imu.isSensorWorking(0)) {
			Quat q = imu.getQuaternion(0);
			bool restCalDone = imu.isRestCalibrationComplete(0);

			Serial.printf(
				"Quat: w=%.3f x=%.3f y=%.3f z=%.3f  RestCal: %s\n",
				q.w,
				q.x,
				q.y,
				q.z,
				restCalDone ? "DONE" : "pending..."
			);
		}
	}
}
