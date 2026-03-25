/*
	SlimeVR Code is placed under the MIT license
	Copyright (c) 2021 Eiren Rain

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

// SlimeIMU Library - Sensor and pin configuration
// Override these defines in your platformio.ini build_flags or before including SlimeIMU.h

#ifndef IMU
#define IMU IMU_AUTO
#endif
#ifndef SECOND_IMU
#define SECOND_IMU IMU_AUTO
#endif
#ifndef BOARD
#define BOARD BOARD_CUSTOM
#endif
#ifndef IMU_ROTATION
#define IMU_ROTATION DEG_0
#endif
#ifndef SECOND_IMU_ROTATION
#define SECOND_IMU_ROTATION DEG_0
#endif

#ifndef PRIMARY_IMU_OPTIONAL
#define PRIMARY_IMU_OPTIONAL false
#endif
#ifndef SECONDARY_IMU_OPTIONAL
#define SECONDARY_IMU_OPTIONAL true
#endif

// Default I2C pins - override via build flags for your board
#ifndef PIN_IMU_SDA
#define PIN_IMU_SDA 21
#endif
#ifndef PIN_IMU_SCL
#define PIN_IMU_SCL 22
#endif
#ifndef PIN_IMU_INT
#define PIN_IMU_INT 255
#endif
#ifndef PIN_IMU_INT_2
#define PIN_IMU_INT_2 255
#endif

// SPI configuration - setting PIN_IMU_CS to anything other than 255
// switches the primary IMU to SPI mode.
// Secondary IMU always uses I2C (PIN_IMU_SDA / PIN_IMU_SCL).
//
// Example platformio.ini build_flags:
//   -DPIN_IMU_CS=5
//   -DPIN_SPI_MOSI=23 -DPIN_SPI_MISO=19 -DPIN_SPI_SCK=18  ; optional, omit for board defaults
//   -DIMU_SPI_CLOCK=24000000                                ; optional, 24 MHz default
#ifndef PIN_IMU_CS
#define PIN_IMU_CS 255  // 255 = I2C mode (default)
#endif
#ifndef IMU_SPI_CLOCK
#define IMU_SPI_CLOCK 24000000  // 24 MHz
#endif
// SPI bus pins - set to 255 to use the board's hardware defaults
#ifndef PIN_SPI_SCK
#define PIN_SPI_SCK 255
#endif
#ifndef PIN_SPI_MISO
#define PIN_SPI_MISO 255
#endif
#ifndef PIN_SPI_MOSI
#define PIN_SPI_MOSI 255
#endif

// BNO08x-specific SPI pins — only needed when using BNO080/085/086 over SPI
#ifndef PIN_BNO_WAK
#define PIN_BNO_WAK 255
#endif
#ifndef PIN_BNO_RST
#define PIN_BNO_RST 255
#endif

#ifndef BATTERY_MONITOR
#define BATTERY_MONITOR BAT_INTERNAL
#endif

#ifndef LED_PIN
#define LED_PIN LED_OFF
#endif
#ifndef LED_INVERTED
#define LED_INVERTED false
#endif
