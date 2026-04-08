# SlimeIMU API Reference

Complete reference for the SlimeIMU public API. For a quick introduction, see the [README](../README.md).

---

## Table of Contents

- [Types](#types)
  - [Quat](#quat)
  - [Vector3](#vector3)
  - [SlimeIMUConfig](#slimeimuconfig)
  - [SensorTypeID](#sensortypeid)
- [Class: SlimeIMU](#class-slimeimu)
  - [Initialization](#initialization)
  - [Data Reading](#data-reading)
  - [Sensor Info](#sensor-info)
  - [Calibration](#calibration)
  - [Advanced](#advanced)
- [Build Flags Reference](#build-flags-reference)
- [Calibration Types](#calibration-types)
- [IMU Selection Macros](#imu-selection-macros)
- [Rotation Constants](#rotation-constants)

---

## Types

### Quat

Quaternion representing 3D orientation. Defined in `lib/math/quat.h`.

```cpp
struct Quat {
    float x, y, z, w;

    Quat();                              // Identity quaternion (0, 0, 0, 1)
    Quat(float x, float y, float z, float w);

    Quat operator*(const Quat& other);   // Quaternion multiplication
    Quat inverse() const;
    float dot(const Quat& other) const;
    Quat normalized() const;
};
```

The identity quaternion `Quat()` represents no rotation: `x=0, y=0, z=0, w=1`.

Returned by [`getQuaternion()`](#getquaternion) when new fused data is available.

### Vector3

3D vector used for acceleration and angular velocity. Defined in `lib/math/vector3.h`.

```cpp
struct Vector3 {
    float x, y, z;

    Vector3();                           // Zero vector (0, 0, 0)
    Vector3(float x, float y, float z);

    Vector3 operator+(const Vector3& other);
    Vector3 operator-(const Vector3& other);
    Vector3 operator*(float scalar);
    float length() const;
    Vector3 normalized() const;
    float dot(const Vector3& other) const;
    Vector3 cross(const Vector3& other) const;
};
```

Returned by [`getLinearAcceleration()`](#getlinearacceleration) (units: m/s²) and [`getAngularVelocity()`](#getangularvelocity) (units: rad/s).

### SlimeIMUConfig

Runtime configuration passed to `SlimeIMU::begin()`. Controls I2C setup and calibration behavior.

```cpp
struct SlimeIMUConfig {
    uint8_t  sdaPin               = 21;       // I2C data pin
    uint8_t  sclPin               = 22;       // I2C clock pin
    uint32_t i2cSpeed             = 400000;   // I2C clock speed in Hz
    bool     useRuntimeCalibration = true;    // Enable continuous background calibration
};
```

| Field | Default | Description |
|---|---|---|
| `sdaPin` | `21` | I2C SDA GPIO pin number |
| `sclPin` | `22` | I2C SCL GPIO pin number |
| `i2cSpeed` | `400000` | I2C bus speed in Hz. 400 kHz (fast mode) is recommended. |
| `useRuntimeCalibration` | `true` | When enabled, the library continuously refines gyro offset calibration in the background during rest periods. Disable if you need fully deterministic behavior. |

**SPI note:** SPI is configured at compile time via build flags, not through this struct. See [Build Flags Reference](#build-flags-reference). When SPI is active, `sdaPin`/`sclPin` still configure the I2C bus for the secondary sensor slot.

### SensorTypeID

Enum identifying the detected sensor hardware. Defined in `src/consts.h`.

```cpp
enum class SensorTypeID : uint8_t {
    Unknown    = 0,
    MPU9250    = 1,
    MPU6500    = 2,
    BNO080     = 3,
    BNO085     = 4,
    BNO055     = 5,
    MPU6050    = 6,
    BNO086     = 7,
    BMI160     = 8,
    ICM20948   = 9,
    ICM42688   = 10,
    BMI270     = 11,
    LSM6DS3TRC = 12,
    LSM6DSV    = 13,
    LSM6DSO    = 14,
    LSM6DSR    = 15,
    ICM45686   = 16,
    ICM45605   = 17,
    ADC_RESISTANCE = 18,
    ISM330DHCX = 19,
    Empty      = 255
};
```

Returned by [`getSensorType()`](#getsensortype). Use [`getSensorName()`](#getsensorname) for a human-readable string.

---

## Class: SlimeIMU

The main library interface. Create one instance and call `begin()` then `update()` in your loop.

```cpp
#include <SlimeIMU.h>

SlimeIMU imu;
```

### Initialization

#### `begin(sdaPin, sclPin)`

```cpp
bool begin(uint8_t sdaPin = 21, uint8_t sclPin = 22);
```

Initialize the library with default settings. Auto-detects connected IMU sensors on the I2C bus.

**Parameters:**
| Param | Type | Default | Description |
|---|---|---|---|
| `sdaPin` | `uint8_t` | `21` | I2C SDA pin |
| `sclPin` | `uint8_t` | `22` | I2C SCL pin |

**Returns:** `true` if at least one working sensor was detected, `false` otherwise.

**Example:**
```cpp
if (!imu.begin(21, 22)) {
    Serial.println("No IMU found!");
    while (true) delay(1000);
}
```

**What happens internally:**
1. Initializes LittleFS and loads saved calibration data
2. Initializes SPI bus if `PIN_IMU_CS` is set (compile-time)
3. Clears and initializes the I2C bus
4. Scans for IMU sensors via WHOAMI registers
5. Configures detected sensors and applies saved calibration
6. Returns whether any sensor is working

Calling `begin()` more than once is safe -- subsequent calls return `true` immediately.

#### `begin(config)`

```cpp
bool begin(const SlimeIMUConfig& config);
```

Initialize with full configuration. See [SlimeIMUConfig](#slimeimuconfig).

**Example:**
```cpp
SlimeIMUConfig config;
config.sdaPin = 8;
config.sclPin = 9;
config.i2cSpeed = 400000;
config.useRuntimeCalibration = true;

if (!imu.begin(config)) {
    Serial.println("No IMU found!");
}
```

---

### Data Reading

#### `update()`

```cpp
void update();
```

Read sensor FIFOs, run sensor fusion, and update all output values. **Call this once per iteration of your `loop()` function.** Does nothing if `begin()` has not been called.

This is non-blocking -- it reads whatever data is available in the sensor's hardware FIFO and processes it. Typical execution time is under 1 ms.

**Example:**
```cpp
void loop() {
    imu.update();
    // ... read data ...
}
```

#### `hasNewData(sensorIndex)`

```cpp
bool hasNewData(size_t sensorIndex) const;
```

Check if the sensor has produced new fused data since the last time it was checked. Use this to avoid processing stale data.

**Parameters:**
| Param | Type | Description |
|---|---|---|
| `sensorIndex` | `size_t` | Sensor index (0 = primary, 1 = secondary) |

**Returns:** `true` if new fused orientation data is available.

**Example:**
```cpp
if (imu.hasNewData(0)) {
    Quat q = imu.getQuaternion(0);
    // process new orientation...
}
```

#### `getQuaternion(sensorIndex)`

```cpp
Quat getQuaternion(size_t sensorIndex) const;
```

Get the fused orientation quaternion for a sensor. The quaternion represents the sensor's absolute orientation in a global reference frame (gravity-aligned, initial heading as reference).

**Parameters:**
| Param | Type | Description |
|---|---|---|
| `sensorIndex` | `size_t` | Sensor index (0 = primary, 1 = secondary) |

**Returns:** `Quat` -- orientation quaternion `(x, y, z, w)`. Returns identity `(0, 0, 0, 1)` if the sensor index is out of range.

**Example:**
```cpp
Quat q = imu.getQuaternion(0);
Serial.printf("w=%.3f x=%.3f y=%.3f z=%.3f\n", q.w, q.x, q.y, q.z);
```

#### `getLinearAcceleration(sensorIndex)`

```cpp
Vector3 getLinearAcceleration(size_t sensorIndex) const;
```

Get linear acceleration with gravity removed.

**Parameters:**
| Param | Type | Description |
|---|---|---|
| `sensorIndex` | `size_t` | Sensor index (0 = primary, 1 = secondary) |

**Returns:** `Vector3` -- acceleration in **m/s²**. Returns `(0, 0, 0)` if the sensor index is out of range.

**Note:** Acceleration data quality depends on accelerometer calibration. Run 6-point calibration (type 3) for best results.

**Example:**
```cpp
Vector3 accel = imu.getLinearAcceleration(0);
float magnitude = accel.length();
if (magnitude > 2.0f) {
    Serial.println("Motion detected!");
}
```

#### `getAngularVelocity(sensorIndex)`

```cpp
Vector3 getAngularVelocity(size_t sensorIndex) const;
```

Get calibrated angular velocity (gyroscope reading with bias removed).

**Parameters:**
| Param | Type | Description |
|---|---|---|
| `sensorIndex` | `size_t` | Sensor index (0 = primary, 1 = secondary) |

**Returns:** `Vector3` -- angular velocity in **rad/s**. Returns `(0, 0, 0)` if the sensor index is out of range.

**Supported sensors:** All softfusion sensors (BMI160, BMI270, ICM42688, ICM45xxx, LSM6DSx, MPU6050) and BNO0xx hardware fusion sensors.

**Example:**
```cpp
Vector3 gyro = imu.getAngularVelocity(0);
Serial.printf("Rotation rate: %.2f rad/s\n", gyro.length());
```

---

### Sensor Info

#### `getSensorCount()`

```cpp
size_t getSensorCount() const;
```

Get the number of sensor slots detected during initialization. This includes both working and non-working sensors (e.g., a secondary slot that found no hardware).

**Returns:** Number of sensor slots (typically 1 or 2).

#### `isSensorWorking(sensorIndex)`

```cpp
bool isSensorWorking(size_t sensorIndex) const;
```

Check if a sensor is initialized and producing data.

**Returns:** `true` if the sensor at the given index is operational.

#### `getSensorType(sensorIndex)`

```cpp
SensorTypeID getSensorType(size_t sensorIndex) const;
```

Get the detected hardware type for a sensor. See [SensorTypeID](#sensortypeid).

**Returns:** `SensorTypeID` enum value. Returns `SensorTypeID::Unknown` for invalid indices.

#### `getSensorName(sensorIndex)`

```cpp
const char* getSensorName(size_t sensorIndex) const;
```

Get a human-readable name for the detected sensor (e.g., `"BMI270"`, `"ICM-42688"`, `"BNO085"`).

**Returns:** Null-terminated string. The pointer is valid for the lifetime of the program.

**Example:**
```cpp
for (size_t i = 0; i < imu.getSensorCount(); i++) {
    if (imu.isSensorWorking(i)) {
        Serial.printf("Sensor %d: %s\n", i, imu.getSensorName(i));
    }
}
```

---

### Calibration

Calibration data is stored in flash via LittleFS and automatically loaded on boot. See the [Calibration Guide](CALIBRATION.md) for practical instructions.

#### `startCalibration(sensorIndex, calibrationType)`

```cpp
void startCalibration(size_t sensorIndex, int calibrationType = 0);
```

Trigger a calibration routine for a specific sensor. The sensor must be working. See [Calibration Types](#calibration-types) for details on each type.

**Parameters:**
| Param | Type | Default | Description |
|---|---|---|---|
| `sensorIndex` | `size_t` | -- | Which sensor to calibrate |
| `calibrationType` | `int` | `0` | Type of calibration to perform (0-4) |

**Important:** Some calibration types require the sensor to be completely still (types 0, 1, 2, 4). Accelerometer calibration (type 3) requires placing the sensor in 6 different orientations. See [Calibration Types](#calibration-types).

**Example:**
```cpp
// Full calibration -- keep sensor still on a flat surface
imu.startCalibration(0, 0);

// Save to flash so it persists across reboots
imu.saveCalibration();
```

#### `saveCalibration()`

```cpp
void saveCalibration();
```

Save all current calibration data to flash (LittleFS). Call this after `startCalibration()` completes, or periodically if using runtime calibration to persist refinements.

Calibration data is automatically loaded from flash on the next `begin()` call.

#### `isRestCalibrationComplete(sensorIndex)`

```cpp
bool isRestCalibrationComplete(size_t sensorIndex) const;
```

Check if the automatic rest calibration has completed for a sensor. Rest calibration runs automatically after `begin()` when the sensor is stationary -- it takes a few seconds.

**Returns:** `true` if rest calibration is done and the sensor is producing calibrated data.

**Example:**
```cpp
// Wait for automatic rest calibration
Serial.println("Keep sensor still...");
while (!imu.isRestCalibrationComplete(0)) {
    imu.update();
    delay(10);
}
Serial.println("Ready!");
```

---

### Advanced

#### `getSensorManager()`

```cpp
SlimeVR::Sensors::SensorManager& getSensorManager();
```

Direct access to the underlying sensor manager. This is an escape hatch for advanced use cases that need access to the internal sensor objects, such as:

- Accessing raw (unfused) sensor data
- Querying sensor-specific status registers
- Implementing custom sensor fusion

**Warning:** The internal API is not stable and may change between versions. Prefer the public `SlimeIMU` methods when possible.

**Example:**
```cpp
auto& manager = imu.getSensorManager();
auto& sensors = manager.getSensors();
for (auto& sensor : sensors) {
    // Access internal sensor objects
}
```

---

## Build Flags Reference

Set these in your `platformio.ini` under `build_flags`:

```ini
build_flags =
  -DPIN_IMU_SDA=21
  -DPIN_IMU_SCL=22
```

### Pin Configuration

| Flag | Default | Description |
|---|---|---|
| `PIN_IMU_SDA` | `21` | I2C data pin |
| `PIN_IMU_SCL` | `22` | I2C clock pin |
| `PIN_IMU_INT` | `255` | Primary IMU interrupt pin (`255` = disabled) |
| `PIN_IMU_INT_2` | `255` | Secondary IMU interrupt pin (`255` = disabled) |
| `LED_PIN` | `LED_OFF` | Status LED pin (`LED_OFF` = disabled) |
| `LED_INVERTED` | `false` | Set `true` if LED is active-low |

### SPI Configuration

Setting `PIN_IMU_CS` to any value other than `255` switches the primary IMU to SPI mode.

| Flag | Default | Description |
|---|---|---|
| `PIN_IMU_CS` | `255` | SPI chip select pin (`255` = I2C mode) |
| `IMU_SPI_CLOCK` | `24000000` | SPI clock speed in Hz |
| `PIN_SPI_SCK` | `255` | SPI clock pin (`255` = board default) |
| `PIN_SPI_MISO` | `255` | SPI MISO pin (`255` = board default) |
| `PIN_SPI_MOSI` | `255` | SPI MOSI pin (`255` = board default) |

### Sensor Configuration

| Flag | Default | Description |
|---|---|---|
| `IMU` | `IMU_AUTO` | Primary IMU type. Set to auto-detect or force a specific sensor. |
| `SECOND_IMU` | `IMU_AUTO` | Secondary IMU type. |
| `IMU_ROTATION` | `DEG_0` | Primary IMU mounting rotation. See [Rotation Constants](#rotation-constants). |
| `SECOND_IMU_ROTATION` | `DEG_0` | Secondary IMU mounting rotation. |
| `PRIMARY_IMU_OPTIONAL` | `false` | If `true`, don't treat a missing primary IMU as fatal. |
| `SECONDARY_IMU_OPTIONAL` | `true` | If `true`, don't treat a missing secondary IMU as fatal. |

---

## Calibration Types

Passed as the `calibrationType` argument to `startCalibration()`:

| Type | Name | Requirements | What it does |
|---|---|---|---|
| `0` | **Full** | Sensor must be still on a flat surface | Runs sample rate calibration, then gyro offset calibration, then accelerometer calibration. Recommended for first-time setup. |
| `1` | **Sample Rate** | Sensor must be still | Measures the actual sensor sample rate and stores it for accurate timing. |
| `2` | **Gyro Offset** | Sensor must be still | Measures gyroscope zero-rate offset at the current temperature. Essential for reducing drift. |
| `3` | **Accelerometer** | Sensor placed in 6 orientations | 6-point accelerometer calibration. Place the sensor on each of its 6 faces (like sides of a die), keeping it still for a few seconds each. Corrects accelerometer scale and offset errors. |
| `4` | **Motionless / CRT** | Sensor must be still | Sensor-specific motionless calibration. On BMI270, this triggers Component Retrimming (CRT) for gyroscope sensitivity correction. |

---

## IMU Selection Macros

Use these with the `IMU` and `SECOND_IMU` build flags:

| Macro | Sensor | Fusion Type |
|---|---|---|
| `IMU_AUTO` | Auto-detect | Automatic |
| `IMU_BMI160` | BMI160 | Software (VQF) |
| `IMU_BMI270` | BMI270 | Software (VQF) |
| `IMU_ICM42688` | ICM-42688 | Software (VQF) |
| `IMU_ICM45605` | ICM-45605 | Software (VQF) |
| `IMU_ICM45686` | ICM-45686 | Software (VQF) |
| `IMU_ISM330DHCX` | ISM330DHCX | Software (VQF) |
| `IMU_LSM6DS3TRC` | LSM6DS3TR-C | Software (VQF) |
| `IMU_LSM6DSO` | LSM6DSO | Software (VQF) |
| `IMU_LSM6DSR` | LSM6DSR | Software (VQF) |
| `IMU_LSM6DSV` | LSM6DSV | Software (VQF) |
| `IMU_MPU6050_SF` | MPU-6050 | Software (VQF) |
| `IMU_BNO055` | BNO055 | Hardware DMP |
| `IMU_BNO080` | BNO080 | Hardware DMP |
| `IMU_BNO085` | BNO085 | Hardware DMP |
| `IMU_BNO086` | BNO086 | Hardware DMP |
| `IMU_ICM20948` | ICM-20948 | Hardware DMP |
| `IMU_MPU6050` | MPU-6050 | Hardware DMP |
| `IMU_MPU6500` | MPU-6500 | Hardware DMP |
| `IMU_MPU9250` | MPU-9250 | Mahony |

---

## Rotation Constants

Use with `IMU_ROTATION` and `SECOND_IMU_ROTATION` build flags to compensate for how the IMU is physically mounted:

| Constant | Angle |
|---|---|
| `DEG_0` | 0° (default, no rotation) |
| `DEG_90` | 90° clockwise |
| `DEG_180` | 180° |
| `DEG_270` | 270° clockwise (= 90° counter-clockwise) |
