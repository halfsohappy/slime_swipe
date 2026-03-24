/**
 * Basic IMU Reading Example
 *
 * Demonstrates how to use the SlimeVR-IMU library to read sensor data
 * from a connected IMU. This example uses the SensorManager to
 * automatically detect and configure IMU sensors, then continuously
 * reads and prints quaternion rotation and acceleration data.
 *
 * Wiring:
 *   Connect your IMU's SDA/SCL pins to the ESP's I2C pins.
 *   Default pins depend on your board (see boards_default.h).
 *   You can override with -DPIN_IMU_SDA=XX -DPIN_IMU_SCL=XX in build flags.
 *
 * Build Flags (platformio.ini):
 *   build_flags =
 *     -O2
 *     -std=gnu++2a
 *     ; Set your IMU type (or use IMU_AUTO for auto-detection):
 *     ; -DIMU=IMU_BMI270
 *     ; -DIMU=IMU_ICM42688
 *     ; -DIMU=IMU_BNO085
 *
 * Supported IMUs:
 *   Software Fusion: BMI160, BMI270, ICM42688, ICM45605, ICM45686,
 *                    LSM6DS3TRC, LSM6DSO, LSM6DSR, LSM6DSV, MPU6050
 *   Hardware DMP:    BNO080, BNO085, BNO086, BNO055, MPU6050, MPU9250,
 *                    ICM20948
 */

#include <SlimeVRIMU.h>

// Required global instances for the library
SlimeVR::Configuration::Configuration configuration;
SlimeVR::Sensors::SensorManager sensorManager;

void setup() {
	Serial.begin(115200);
	while (!Serial) {
		delay(10);
	}
	delay(1000);

	Serial.println("SlimeVR-IMU Library - Basic IMU Example");
	Serial.println("========================================");

	// Initialize filesystem for calibration storage
	configuration.setup();

	// Detect and initialize all configured sensors
	sensorManager.setup();
	sensorManager.postSetup();

	Serial.println("Sensor setup complete!");

	// Print detected sensors
	auto& sensors = sensorManager.getSensors();
	Serial.printf("Found %d sensor(s)\n", sensors.size());
	for (size_t i = 0; i < sensors.size(); i++) {
		auto& sensor = sensors[i];
		Serial.printf(
			"  Sensor %d: %s [%s]\n",
			i,
			getIMUNameByType(sensor->getSensorType()),
			sensor->isWorking() ? "OK" : "ERROR"
		);
	}
	Serial.println();
}

void loop() {
	// Update all sensors (reads new IMU data and runs fusion)
	sensorManager.update();

	// Read data from each sensor
	auto& sensors = sensorManager.getSensors();
	for (size_t i = 0; i < sensors.size(); i++) {
		auto& sensor = sensors[i];
		if (!sensor->isWorking()) {
			continue;
		}

		// Get fused rotation as quaternion (w, x, y, z)
		const Quat& rotation = sensor->getFusedRotation();

		// Get linear acceleration (m/s^2)
		const Vector3& accel = sensor->getAcceleration();

		Serial.printf(
			"[Sensor %d] Quat: w=%.4f x=%.4f y=%.4f z=%.4f | "
			"Accel: x=%.2f y=%.2f z=%.2f m/s²\n",
			i,
			rotation.w,
			rotation.x,
			rotation.y,
			rotation.z,
			accel.x,
			accel.y,
			accel.z
		);
	}

	delay(10);  // ~100Hz output rate
}
