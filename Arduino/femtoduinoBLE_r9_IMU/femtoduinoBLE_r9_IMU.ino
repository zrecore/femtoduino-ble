/**
 * Transmit FreeIMU data via BLE113 module using the FemtoduinoBLE!
 * 
 * Target board: 
 *   FemtoduinoBLE (revision 9) or higher.
 *
 * NOTE:
 *   If you intend to use FemtoduinoBLE revision 8, you will need to unsolder the 
 *   TX solder jumper, cut the trace between the solder jumper pads only, and solder
 *   a jumper wire from D9 to the BLE end of that solder jumper.
 *
 * Setup:
 *   Hook up a FreeIMU/FemtoIMU to your FemtoduinoBLE as follows:
 *
 *   1) FreeIMU SDA pin or FemtoIMU A4 pin  ---> FemtoduinoBLE (r9 or higher) pin D2
 *   2) FreeIMU SCL pin or FemtoIMU A5 pin  ---> FemtoduinoBLE (r9 or higher) pin D3
 *
 *   3) FreeIMU VIN pin or FemtoIMU VIN pin ---> FemtoduinoBLE (r9 or higher) pin VIN
 *
 *   4) Connect all GND pins to common ground.
 * 
 */
 
/**
 * BGLib code picked up from Jeff Rowberg's Arduino + BGLib examples
 */
  
/**
 * FreeIMU libraries originally by F.Varesano.
 */

#include <ADXL345.h>
#include <bma180.h>
#include <HMC58X3.h>
#include <ITG3200.h>
#include <MS561101BA.h>
#include <I2Cdev.h>
#include <MPU60X0.h>
#include <EEPROM.h>

//#define DEBUG
#include "DebugUtils.h"
#include "FreeIMU.h"
#include <Wire.h>
#include <SPI.h>

#include <math.h>

float ypr[3]; // yaw pitch roll

// Set the FreeIMU object
FreeIMU my3IMU = FreeIMU();


void setup() {
  Mouse.begin();
  //Serial.begin(115200);
  Wire.begin();
  
  my3IMU.init(true);
}


void loop() {
  
  my3IMU.getYawPitchRoll(ypr);
  
  // scale angles to mouse movements. You can replace 10 with whatever feels adequate for you.
  // biggere values mean faster movements
  int x = map(ypr[1], -90, 90, -10, 10);
  int y = map(ypr[2], -90, 90, -10, 10);
  
  /**
  // move mouse
  Mouse.move(-x, y, 0);
  **/
  
  // Transmit data to other BLE device
}



