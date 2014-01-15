/**
 * @author Alexander Entinger, MSc / LXRobotics
 * @brief this file implements the reading of the ldr values from the LXR_Tubeclock_Shield library
 * @file ldr_read_serial.ino
 * @license Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0) ( http://creativecommons.org/licenses/by-nc-sa/3.0/ )
 */

#include "LXR_Tubeclock_Shield.h"
#include <util/delay.h> // for _delay_ms

void setup() {
  LXR_Tubeclock_Shield::begin(); // initialize library
  Serial.begin(115200); // initialize serial interface
}

void loop() {
  Serial.print("LDR = ");
  Serial.println(LXR_Tubeclock_Shield::get_ldr_voltage(), DEC);
  _delay_ms(500); 
}

