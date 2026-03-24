"""
Build script for SlimeVR-IMU library.

When this project is used as a PlatformIO library, this script adds the
internal lib/ subdirectories (VQF, math, sensor drivers, etc.) as
additional build sources and include paths so they compile alongside
the main library code in src/.
"""

Import("env")
import os
import glob as globmod

# Library root directory (where this script lives)
root = os.path.dirname(os.path.realpath(__file__))

# Internal library paths: (build_name, relative_path)
internal_libs = [
    ("vqf", "lib/vqf"),
    ("math", "lib/math"),
    ("magneto", "lib/magneto"),
    ("magnetometers", "lib/magnetometers"),
    ("i2cdev", "lib/i2cdev"),
    ("i2cscan", "lib/i2cscan"),
    ("ICM20948", "lib/ICM20948"),
    ("bno055_adafruit", "lib/bno055_adafruit"),
    ("bno080", "lib/bno080"),
    ("mpu6050", "lib/mpu6050"),
    ("mpu9250", "lib/mpu9250"),
    ("mpu6050offsetfinder", "lib/mpu6050offsetfinder"),
]

for name, rel_path in internal_libs:
    full_path = os.path.join(root, rel_path)
    if not os.path.isdir(full_path):
        continue

    # Add as include path
    env.Append(CPPPATH=[full_path])

    # Check if there are source files to compile
    sources = (
        globmod.glob(os.path.join(full_path, "*.cpp"))
        + globmod.glob(os.path.join(full_path, "*.c"))
    )
    if sources:
        env.BuildSources(
            os.path.join("$BUILD_DIR", "slimevr_libs", name),
            full_path,
        )
