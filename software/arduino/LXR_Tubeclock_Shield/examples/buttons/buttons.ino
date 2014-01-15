/**
 * @author Alexander Entinger, MSc  / LXRobotics
 * @brief this file demonstrates how to use the buttons with the LXR_Tubeclock_Shield library
 * @file buttons.ino
 * @license Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0) ( http://creativecommons.org/licenses/by-nc-sa/3.0/ )
 */

#include "LXR_Tubeclock_Shield.h"
#include <util/delay.h> // for _delay_ms

void setup() {
  // initialize the library
  LXR_Tubeclock_Shield::begin();
  // register the button callbacks
  LXR_Tubeclock_Shield::register_button_callback(BUTTON1, &button1_pressed);
  LXR_Tubeclock_Shield::register_button_callback(BUTTON2, &button2_pressed);
  // start serial interface
  Serial.begin(115200);
}

static boolean button_1_pressed = false;
static boolean button_2_pressed = false;
/**
 * @brief callback functions in case of button events - run in interrupt context
 */
void button1_pressed() {
  button_1_pressed = true;
}
void button2_pressed() {
  button_2_pressed = true;
}

void loop() {
  if(button_1_pressed) {
     Serial.println("Button 1 pressed");
     _delay_ms(250); // for debouncing, dont use "delay" since it needs timer 0 and that one is used by the LXR_Tubeclock_Shield library 
     button_1_pressed = false; // clear flag
  }
  if(button_2_pressed) {
     Serial.println("Button 2 pressed");
     _delay_ms(250);
     button_2_pressed = false;
  }
}


