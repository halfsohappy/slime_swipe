# SlimeIMU

High-quality IMU sensor fusion library for ESP32, extracted from [SlimeVR](https://slimevr.dev) firmware.

Get fused orientation quaternions, linear acceleration, and angular velocity from 15+ supported IMUs with automatic detection, VQF sensor fusion, and persistent calibration -- all behind a simple API.

## Features

- **VQF sensor fusion** -- state-of-the-art quaternion filter, better than Mahony/Madgwick for most applications
- **Automatic IMU detection** via WHOAMI registers -- no need to specify which sensor you have
- **15 supported IMUs** across software fusion and hardware DMP
- **Calibration with flash persistence** (LittleFS) -- calibrate once, data survives reboots
- **Gyroscope temperature compensation** -- drift is corrected as the sensor warms up
- **I2C and SPI** interfaces, with I2C multiplexer (PCA9547) and GPIO expander (MCP23X17) support
- **Multi-sensor** -- up to 2 sensors on the same bus with automatic address detection

## Supported IMUs

| IMU | Fusion | Interface | Notes |
|-----|--------|-----------|-------|
| BMI160 | Software (VQF) | I2C / SPI | Optional HMC5883L / QMC5883L magnetometer |
| BMI270 | Software (VQF) | I2C / SPI | Gyro sensitivity auto-calibration (CRT) |
| ICM-42688 | Software (VQF) | I2C / SPI | |
| ICM-45605 | Software (VQF) | I2C / SPI | |
| ICM-45686 | Software (VQF) | I2C / SPI | |
| LSM6DS3TR-C | Software (VQF) | I2C | |
| LSM6DSO | Software (VQF) | I2C | |
| LSM6DSR | Software (VQF) | I2C | |
| LSM6DSV | Software (VQF) | I2C | |
| MPU-6050 | Software (VQF) | I2C | Use `IMU_MPU6050_SF` for software fusion variant |
| BNO055 | Hardware DMP | I2C | Not recommended (BNO080+ is much better) |
| BNO080 | Hardware DMP | I2C | Good results with internal fusion |
| BNO085 | Hardware DMP | I2C | Best results with ARVR stabilization |
| BNO086 | Hardware DMP | I2C | Same as BNO085 |
| ICM-20948 | Hardware DMP | I2C / SPI | 6DoF or 9DoF mode |
| MPU-6050 | Hardware DMP | I2C | Legacy DMP mode (`IMU_MPU6050`) |
| MPU-6500 | Hardware DMP | I2C | Legacy DMP mode |
| MPU-9250 | Mahony | I2C | Requires magnetometer calibration |

## Installation

### PlatformIO (recommended)

Add to your `platformio.ini`:

```ini
lib_deps =
  https://github.com/halfsohappy/slime_swipe.git

build_flags =
  -DPIN_IMU_SDA=21
  -DPIN_IMU_SCL=22
```

### Manual

Clone this repository into your project's `lib/` directory:

```bash
cd your-project/lib/
git clone https://github.com/halfsohappy/slime_swipe.git SlimeIMU
```

## Quick Start

```cpp
#include <Arduino.h>
#include <SlimeIMU.h>

SlimeIMU imu;

void setup() {
    Serial.begin(115200);

    if (!imu.begin(21, 22)) {  // SDA, SCL
        Serial.println("No IMU found!");
        while (true) delay(1000);
    }

    Serial.printf("Detected: %s\n", imu.getSensorName(0));
}

void loop() {
    imu.update();

    if (imu.hasNewData(0)) {
        Quat q = imu.getQuaternion(0);
        Vector3 accel = imu.getLinearAcceleration(0);

        Serial.printf("w=%.3f x=%.3f y=%.3f z=%.3f  accel: %.2f %.2f %.2f\n",
                      q.w, q.x, q.y, q.z, accel.x, accel.y, accel.z);
    }
}
```

## Configuration

All hardware configuration is done via build flags in `platformio.ini`. The library uses sensible defaults -- you only need to set the pins for your board.

### Pin Configuration

| Build Flag | Default | Description |
|---|---|---|
| `PIN_IMU_SDA` | `21` | I2C data pin |
| `PIN_IMU_SCL` | `22` | I2C clock pin |
| `PIN_IMU_INT` | `255` (disabled) | Primary IMU interrupt pin |
| `PIN_IMU_INT_2` | `255` (disabled) | Secondary IMU interrupt pin |
| `LED_PIN` | `LED_OFF` | Status LED pin (set to `LED_OFF` to disable) |

### SPI Mode

Set `PIN_IMU_CS` to a valid GPIO to switch the primary IMU to SPI. The secondary sensor slot always uses I2C.

```ini
build_flags =
  -DPIN_IMU_CS=5
  -DPIN_SPI_SCK=18       ; optional, omit for board defaults
  -DPIN_SPI_MISO=19      ; optional
  -DPIN_SPI_MOSI=23      ; optional
  -DIMU_SPI_CLOCK=24000000  ; optional, default 24 MHz
```

### Sensor Selection

By default, the library auto-detects sensors (`IMU_AUTO`). To force a specific sensor:

```ini
build_flags =
  -DIMU=IMU_BMI270
  -DSECOND_IMU=IMU_BMI270
```

### Sensor Rotation

If your IMU is mounted rotated relative to your project's coordinate frame:

```ini
build_flags =
  -DIMU_ROTATION=DEG_90         ; DEG_0, DEG_90, DEG_180, DEG_270
  -DSECOND_IMU_ROTATION=DEG_180
```

## Calibration

For best results, place the sensor on a flat surface and keep it still for a few seconds after power-on. The library performs automatic rest calibration during this time.

For full calibration (gyro offset + accelerometer 6-point), see the [calibration example](examples/calibration/main.cpp) and the [Calibration Guide](docs/CALIBRATION.md).

Calibration data is automatically saved to flash (LittleFS) and loaded on subsequent boots.

## Examples

| Example | Description |
|---|---|
| [basic_imu](examples/basic_imu/main.cpp) | Read orientation and acceleration from a single sensor |
| [calibration](examples/calibration/main.cpp) | Interactive calibration via serial commands |
| [multi_sensor](examples/multi_sensor/main.cpp) | Two sensors on the same I2C bus with Euler angle output |

## API Reference

See [docs/API.md](docs/API.md) for the full API reference covering all public types and methods.

## License

Dual-licensed under [MIT](LICENSE-MIT) or [Apache 2.0](LICENSE-APACHE), at your option.

Based on [SlimeVR firmware](https://github.com/SlimeVR/SlimeVR-Tracker-ESP) by Eiren Rain & SlimeVR contributors.
