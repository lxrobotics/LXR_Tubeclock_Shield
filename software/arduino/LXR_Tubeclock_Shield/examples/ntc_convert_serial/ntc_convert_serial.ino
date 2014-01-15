/**
 * @author Alexander Entinger, MSc / LXRobotics
 * @brief this file implements the reading of the ntc values from the LXR_Tubeclock_Shield library and converting them to Celsius using the NTCLib
 * @file ntc_convert_serial.ino
 * @license Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0) ( http://creativecommons.org/licenses/by-nc-sa/3.0/ )
 */

#include "LXR_Tubeclock_Shield.h"
#include "NTCLib.h"
#include <util/delay.h> // for _delay_ms

void setup() {
  LXR_Tubeclock_Shield::begin(); // initialize library
  Serial.begin(115200); // initialize serial interface
}

void loop() {
  Serial.print("Temperatur = ");
  Serial.print(NTCLib::convert(LXR_Tubeclock_Shield::get_ntc_voltage()), DEC);
  Serial.println(" °C");
  _delay_ms(500); 
}

