/** 
 * @author Alexander Entinger MSc, LXRobotics
 * @brief this file demonstrates how to use the IV18 library in combination with the LXR Tubeclock shield (+ LXR_Tubeclock_Shield Library)
 * @file iv18_tubeclock_demo.ino
 * @license Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0) ( http://creativecommons.org/licenses/by-nc-sa/3.0/ )
 */

#include "LXR_Tubeclock_Shield.h"
#include "LXR_Tubeclock_Shield_IV18.h"
#include "NTCLib.h"
#include <Wire.h> // necessary for RTClib
#include "RTClib.h"

// uncomment this define, if you want to see the value read from the adc for the ldr and for the ntc
//#define OUTPUT_LDR_AND_NTC_TO_SERIAL

uint8_t display_content[DISPLAY_SIZE] = {
  ' ',' ',' ',' ',' ',' ',' ',' '};

RTC_DS1307 RTC;

void setup() {
  LXR_Tubeclock_Shield::begin(); // needs always to be called first
  LXR_Tubeclock_Shield_IV18::begin(); // needs to be after XR_Tubeclock_Shield::begin()
  
  Wire.begin();
  RTC.begin(); 
  RTC.adjust(DateTime(__DATE__, __TIME__)); // sets the RTC to the date & time this sketch was compiled
  RTC.setSqwOutSignal(RTC_DS1307::Frequency_1Hz); // set output frequency to 1 Hz
  RTC.register_callback(&tick, EINT0);

  // write the empty string to the display
  LXR_Tubeclock_Shield_IV18::write_string(display_content);
  
  // register the button callbacks
  LXR_Tubeclock_Shield::register_button_callback(BUTTON1, &button1_pressed);
  LXR_Tubeclock_Shield::register_button_callback(BUTTON2, &button2_pressed);
  
  // start serial
  Serial.begin(115200);
}

static boolean button_1_pressed = false;
static boolean button_2_pressed = false;
/**
 * @brief callback functions in case of button events
 */
void button1_pressed() {
  button_1_pressed = true;
}
void button2_pressed() {
  button_2_pressed = true;
}

static boolean update_display = false;
/**
 * @brief this function is executed once per second
 */
void tick() {
  update_display = true;
}

static uint8_t temp_display_cnt = 0; // cnts up, if it reaches max_temp_display_cnt its reset and the temperature is shown for one second
static uint8_t const max_temp_display_cnt = 10;

void loop() {
  if(button_1_pressed) {
   Serial.println("Button 1 pressed"); 
   button_1_pressed = false;
  } 
  if(button_2_pressed) {
   Serial.println("Button 2 pressed"); 
   button_2_pressed = false;
  }
  if(update_display) {
    update_display = false;

    temp_display_cnt++;
    if(temp_display_cnt >= max_temp_display_cnt) {
      temp_display_cnt = 0;
      int const temperature = NTCLib::convert(LXR_Tubeclock_Shield::get_ntc_voltage());
      sprintf((char*)(display_content), "        ", temperature); // clear display
      sprintf((char*)(display_content), "%d*C", temperature); // write the temperature in it
    } 
    else {
      DateTime now = RTC.now();
      // convert to a display string
      if(now.hour() < 10) {
        display_content[0] = '0';
        sprintf((char*)(display_content + 1), "%d", now.hour());
      } 
      else {
        sprintf((char*)(display_content), "%d", now.hour());      
      }
      if(now.minute() < 10) {
        display_content[3] = '0';
        sprintf((char*)(display_content + 4), "%d", now.minute());
      } 
      else {
        sprintf((char*)(display_content + 3), "%d", now.minute()); 
      }
      if(now.second() < 10) {
        display_content[6] = '0';
        sprintf((char*)(display_content + 7), "%d", now.second());
      } 
      else {
        sprintf((char*)(display_content + 6), "%d", now.second());
      }
    }

    // write time to display
    LXR_Tubeclock_Shield_IV18::write_string(display_content);
    
#ifdef OUTPUT_LDR_AND_NTC_TO_SERIAL
    Serial.print("LDR = ");
    Serial.println(LXR_Tubeclock_Shield::get_ldr_voltage(), DEC);
    Serial.print("NTC = ");
    Serial.println(LXR_Tubeclock_Shield::get_ntc_voltage(), DEC);
#endif
  }
}



