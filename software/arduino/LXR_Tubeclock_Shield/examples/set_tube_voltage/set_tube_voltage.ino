/**
 * @author Alexander Entinger, MSc / LXRobotics
 * @brief this file demonstrates how to set and enable/disable the tube voltage generation
 * @file set_tube_voltage.ino
 * @license Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0) ( http://creativecommons.org/licenses/by-nc-sa/3.0/ )
 */

#include "LXR_Tubeclock_Shield.h"

void setup() {
  LXR_Tubeclock_Shield::begin(); // initialize library
  LXR_Tubeclock_Shield::set_tube_voltage(25); // set tube voltage to 25 V (max 69 V)
  LXR_Tubeclock_Shield::enable_tube_voltage(); // start step up converter to generate tube voltage
  // LXR_Tubeclock_Shield::disable_tube_voltage(); // stop step up converter to generate tube voltage
}

void loop() {

}


