# SlimeIMU

High-quality IMU sensor fusion library for ESP32, extracted from SlimeVR code.

This repository now provides a reusable Arduino/PlatformIO library (not a full tracker
firmware project). It focuses on:

- automatic IMU detection
- I2C and SPI transport support
- quaternion and linear acceleration output
- runtime calibration workflows
- persistent calibration storage (LittleFS)

## Installation (PlatformIO)

Add the library to your project's `lib_deps`:

```ini
[env:your_env]
platform = espressif32
framework = arduino
lib_deps =
  https://github.com/halfsohappy/slime_swipe.git
```

Then include:

```cpp
#include <SlimeIMU.h>
```

## Quick start

```cpp
#include <Arduino.h>
#include <SlimeIMU.h>

SlimeIMU imu;

void setup() {
  Serial.begin(115200);
  if (!imu.begin(21, 22)) {
    Serial.println("No IMU detected");
    while (true) delay(1000);
  }
}

void loop() {
  imu.update();

  if (imu.hasNewData(0)) {
    Quat q = imu.getQuaternion(0);
    Vector3 a = imu.getLinearAcceleration(0);
    Serial.printf("Quat w=%.3f x=%.3f y=%.3f z=%.3f | Accel x=%.2f y=%.2f z=%.2f\n",
                  q.w, q.x, q.y, q.z, a.x, a.y, a.z);
  }
}
```

## SPI usage

SPI is supported through build flags in your PlatformIO environment:

```ini
build_flags =
  -DIMU_PROTOCOL=IMU_PROTOCOL_SPI
  -DPIN_IMU_CS=5
  -DPIN_IMU_CS_2=255
  -DPIN_IMU_INT=255
  -DPIN_IMU_INT_2=255
```

Optional SPI settings:

- `IMU_SPI_CLOCK` (default `24000000UL`)
- `IMU_SPI_BIT_ORDER` (default `MSBFIRST`)
- `IMU_SPI_DATA_MODE` (default `SPI_MODE3`)
- `PIN_IMU_SPI_SCK`, `PIN_IMU_SPI_MISO`, `PIN_IMU_SPI_MOSI` (optional runtime SPI pin setup)

## Public API

Main class: `SlimeIMU`

- `bool begin(uint8_t sdaPin = 21, uint8_t sclPin = 22);`
- `bool begin(const SlimeIMUConfig& config);`
- `void update();`
- `size_t getSensorCount() const;`
- `bool isSensorWorking(size_t sensorIndex) const;`
- `bool hasNewData(size_t sensorIndex) const;`
- `Quat getQuaternion(size_t sensorIndex) const;`
- `Vector3 getLinearAcceleration(size_t sensorIndex) const;`
- `SensorTypeID getSensorType(size_t sensorIndex) const;`
- `const char* getSensorName(size_t sensorIndex) const;`
- `void startCalibration(size_t sensorIndex, int calibrationType = 0);`
- `void saveCalibration();`
- `bool isRestCalibrationComplete(size_t sensorIndex) const;`

Calibration types for `startCalibration(...)`:

- `0` = all
- `1` = sample rate
- `2` = gyro offset
- `3` = accelerometer
- `4` = motionless

## Supported IMUs

Software fusion:

- BMI160, BMI270, ICM42688, ICM45605, ICM45686
- LSM6DS3TRC, LSM6DSO, LSM6DSR, LSM6DSV
- MPU6050

Hardware fusion:

- BNO055, BNO080, BNO085, BNO086
- ICM20948

## Examples

See `/examples`:

- `examples/basic_imu/main.cpp`
- `examples/calibration/main.cpp`
- `examples/multi_sensor/main.cpp`

These examples are the recommended starting point for integration.

## Linux serial permissions

If upload/monitor fails due to permissions, see PlatformIO udev rules:
https://docs.platformio.org/en/latest/faq.html#platformio-udev-rules


## Contributions
Any contributions submitted for inclusion in this repository will be dual-licensed under
either:

- MIT License ([LICENSE-MIT](/LICENSE-MIT))
- Apache License, Version 2.0 ([LICENSE-APACHE](/LICENSE-APACHE))

Unless you explicitly state otherwise, any contribution intentionally submitted for
inclusion in the work by you, as defined in the Apache-2.0 license, shall be dual
licensed as above, without any additional terms or conditions.

You also certify that the code you have used is compatible with those licenses or is
authored by you. If you're doing so on your work time, you certify that your employer is
okay with this and that you are authorized to provide the above licenses.

For an explanation on how to contribute, see [`CONTRIBUTING.md`](CONTRIBUTING.md)
