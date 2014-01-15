/** 
 * @author Alexander Entinger MSc, LXRobotics
 * @brief this file demonstrates how to use the IV18 library in combination with the LXR Tubeclock shield (+ LXR_Tubeclock_Shield Library)
 * @file iv18_demo.ino
 * @license Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0) ( http://creativecommons.org/licenses/by-nc-sa/3.0/ )
 */

#include "LXR_Tubeclock_Shield.h"
#include "LXR_Tubeclock_Shield_IV18.h"

uint8_t display_content[DISPLAY_SIZE] = {
  '1','2','3','4','5','6','7','8'};

void setup() {
  LXR_Tubeclock_Shield::begin(); // needs always to be called first
  LXR_Tubeclock_Shield_IV18::begin(); // needs to be after XR_Tubeclock_Shield::begin()

  // write the empty string to the display
  LXR_Tubeclock_Shield_IV18::write_string(display_content);
}

void loop() {

}



