# Calibration Guide

This guide explains how calibration works in SlimeIMU and how to get the best results from your sensors.

---

## How Calibration Works

SlimeIMU uses the **VQF (Versatile Quaternion-based Filter)** for sensor fusion on software-fusion IMUs. VQF combines gyroscope and accelerometer data to produce stable orientation estimates, but it needs accurate calibration data to perform well.

Calibration data includes:
- **Gyroscope zero-rate offset** -- the small bias each gyro axis reports when stationary
- **Gyroscope temperature coefficients** -- how the offset changes as the sensor warms up
- **Accelerometer scale and offset** -- corrects manufacturing imperfections in the accelerometer
- **Sample rate** -- the actual sensor output rate (may differ slightly from the nominal rate)

All calibration data is stored in **LittleFS** (flash filesystem on the ESP32) and automatically loaded when `imu.begin()` is called. Calibration survives power cycles and reboots.

---

## Rest Calibration (Automatic)

Every time the library starts, it performs **automatic rest calibration**:

1. Call `imu.begin()` and `imu.update()` in your loop
2. Keep the sensor stationary on a flat surface
3. The library detects rest and refines the gyro offset
4. Check completion with `imu.isRestCalibrationComplete(sensorIndex)`

This takes a few seconds and happens transparently. It does not require any user interaction.

```cpp
// Wait for automatic rest calibration
Serial.println("Keep sensor still...");
while (!imu.isRestCalibrationComplete(0)) {
    imu.update();
    delay(10);
}
Serial.println("Calibrated and ready!");
```

**Tip:** For best results, power on your device and let it sit undisturbed for 3-5 seconds before moving it.

---

## Runtime Calibration (Continuous)

When `useRuntimeCalibration` is enabled (the default), the library continuously refines gyro calibration in the background whenever the sensor is at rest. This means:

- Gyro drift improves over time as the sensor warms up
- Temperature compensation coefficients are updated automatically
- No explicit calibration calls are needed for basic use

To persist runtime calibration improvements, call `imu.saveCalibration()` periodically (e.g., every few minutes or on shutdown).

---

## Manual Calibration

For the best accuracy, run a full manual calibration at least once per sensor. The [calibration example](../examples/calibration/main.cpp) provides an interactive serial interface.

### Full Calibration (Type 0)

Runs all sub-calibrations in sequence: sample rate, gyro offset, and accelerometer.

```cpp
// Keep sensor perfectly still on a flat surface
imu.startCalibration(0, 0);
imu.saveCalibration();
```

### Gyro Offset Calibration (Type 2)

Measures the gyroscope zero-rate offset at the current temperature. This is the most important calibration for reducing drift.

```cpp
// Keep sensor perfectly still
imu.startCalibration(0, 2);
imu.saveCalibration();
```

**Tip:** For best results, calibrate after the sensor has warmed up for a few minutes. Gyro offset is temperature-dependent, and calibrating at operating temperature gives the most accurate result.

### Accelerometer 6-Point Calibration (Type 3)

Corrects accelerometer scale and offset errors by measuring gravity in 6 orientations.

```cpp
imu.startCalibration(0, 3);
imu.saveCalibration();
```

During this calibration, you need to:
1. Place the sensor flat (chip up) -- hold still for ~3 seconds
2. Flip it upside down (chip down) -- hold still for ~3 seconds
3. Place it on its left side -- hold still for ~3 seconds
4. Place it on its right side -- hold still for ~3 seconds
5. Stand it on one end -- hold still for ~3 seconds
6. Stand it on the other end -- hold still for ~3 seconds

The sensor auto-detects when each position is stable. The order of orientations does not matter, but each must be a distinct axis direction.

### Sample Rate Calibration (Type 1)

Measures the actual sensor output rate. Usually only needed once.

```cpp
imu.startCalibration(0, 1);
imu.saveCalibration();
```

### Motionless / CRT Calibration (Type 4)

Sensor-specific motionless calibration. On the **BMI270**, this triggers **Component Retrimming (CRT)**, which corrects gyroscope sensitivity errors. On other sensors, this is equivalent to gyro offset calibration.

```cpp
// Keep sensor perfectly still
imu.startCalibration(0, 4);
imu.saveCalibration();
```

---

## Per-IMU Notes

### BNO080 / BNO085 / BNO086

These sensors use their internal **hardware DMP** for sensor fusion. Calibration is handled internally by the BNO's firmware:

- The BNO continuously auto-calibrates while in use
- No explicit `startCalibration()` calls are needed
- Calibration status is tracked internally by the sensor
- The library still saves/loads BNO calibration data to flash

### ICM-20948

Uses the internal DMP for fusion. Supports both 6DoF and 9DoF modes:

- **6DoF mode** (default): Uses gyro + accelerometer only. Good in all environments.
- **9DoF mode**: Also uses the magnetometer. Requires a magnetically clean environment. Enable by commenting out `USE_6DOF` in `debug.h`.

The ICM-20948 continuously refines its bias estimates. The library periodically saves these to flash.

### BMI270

Supports all calibration types plus **CRT (Component Retrimming)** via type 4. CRT corrects manufacturing variations in gyroscope sensitivity and is recommended during initial setup.

For best results:
1. Let the sensor warm up for 2-3 minutes
2. Run type 4 (CRT) calibration
3. Run type 0 (full) calibration
4. Save with `imu.saveCalibration()`

### BMI160

Supports all standard calibration types. Optional magnetometer support (HMC5883L or QMC5883L) for 9-axis fusion.

### Software Fusion Sensors (General)

All software fusion sensors (BMI160, BMI270, ICM42688, ICM45xxx, LSM6DSx, MPU6050 SF) share the same calibration framework:

- **Gyro offset calibration** is the most impactful -- run it at least once
- **Accelerometer 6-point calibration** improves pitch/roll accuracy
- **Runtime calibration** handles gradual temperature drift automatically
- Calibration data is per-sensor -- if you swap IMU hardware, recalibrate

### MPU-6050 / MPU-6500 / MPU-9250 (Hardware DMP)

These legacy sensors use the internal DMP for fusion:

- **MPU-6050/6500**: Auto-calibrates on power-on. Keep the sensor still on a flat surface for a few seconds after booting.
- **MPU-9250**: Requires magnetometer calibration. On first boot, rotate the sensor in a figure-8 pattern for ~30 seconds when the LED blinks.

---

## Resetting Calibration

To clear all saved calibration data, erase the LittleFS partition:

```cpp
#include <LittleFS.h>

void resetCalibration() {
    LittleFS.begin(true);
    LittleFS.format();
    Serial.println("Calibration data erased. Reboot to recalibrate.");
    ESP.restart();
}
```

Alternatively, you can use the PlatformIO upload filesystem command to flash an empty filesystem image.

---

## Recommended Calibration Workflow

For a new sensor:

1. **Mount the sensor** in its final orientation
2. **Set `IMU_ROTATION`** in your build flags if the sensor is rotated
3. **Power on** and wait 3-5 seconds for rest calibration (automatic)
4. **Run full calibration** (type 0) with the sensor on a flat surface
5. **Run 6-point accel calibration** (type 3) if you need accurate pitch/roll
6. **Run CRT** (type 4) if using BMI270
7. **Save** with `imu.saveCalibration()`
8. **Reboot** -- calibration will be loaded automatically

After this initial setup, runtime calibration handles ongoing drift correction. You generally don't need to recalibrate unless you replace the sensor hardware.
