/** 
 * @author Alexander Entinger MSc, LXRobotics
 * @brief this file demonstrates how to use the LXR_Tubeclock_Shield library to control a IV6 tube
 * @file iv6_demo.ino
 * @license Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0) ( http://creativecommons.org/licenses/by-nc-sa/3.0/ )
 */

#include "LXR_Tubeclock_Shield.h"
#include <util/delay.h> // for _delay_ms, dont use delay here, since it needs timer 0 and the tube clock shields needs that too

void setup() {
  // initialize the tube clock shield
  LXR_Tubeclock_Shield::begin(); 
  // set the tube voltage to 25 V
  LXR_Tubeclock_Shield::set_tube_voltage(25);
  // enable the tube voltage generation
  LXR_Tubeclock_Shield::enable_tube_voltage();
  // enable the vfd controller
  LXR_Tubeclock_Shield::enable_vfd_controller();
  // write 0 to all segments
  s_segment_map seg_map; 
  seg_map.all = 0;
  LXR_Tubeclock_Shield::vfd_controller_write(seg_map);
  // load the written data into the display latches, a write command always goes in combination with a load command, otherwise the written information is not stored
  LXR_Tubeclock_Shield::vfd_controller_load();
  // disable the blank so that the written data can be shown - since all digits are off the display is blank
  LXR_Tubeclock_Shield::vfd_controller_blank_off();	
}

void loop() {
  for(uint8_t digit_value = 0; digit_value <= 10; digit_value++) {
    iv6_set_digit(digit_value);
    _delay_ms(250); 
  }
}

// defines 
#define DP		(1<<0)
#define A		(1<<1)
#define B		(1<<2)
#define C		(1<<3)
#define D		(1<<4)
#define E		(1<<5)
#define F		(1<<6)
#define G		(1<<7)
#define	DIG_NUM_0	(A | B | C | D | E | F)
#define	DIG_NUM_1	(B | C)
#define DIG_NUM_2       (A | B | G | E | D)
#define DIG_NUM_3	(A | B | C | D | G)
#define DIG_NUM_4	(B | C | F | G)
#define DIG_NUM_5	(A | F | G | C | D)
#define DIG_NUM_6	(A | F | E | D | C | G)
#define DIG_NUM_7	(A | B | C)
#define DIG_NUM_8	(A | B | C | D | E | F | G)
#define DIG_NUM_9	(A | B | C | D | G | F)
#define DIG_BLANK	(0)

// global variables
static volatile uint8_t const digit_map[] = {
  DIG_NUM_0, DIG_NUM_1, DIG_NUM_2, DIG_NUM_3, DIG_NUM_4, DIG_NUM_5, DIG_NUM_6, DIG_NUM_7, DIG_NUM_8, DIG_NUM_9, DIG_BLANK
}; 

/** 
 * @brief this function sets a digit of the iv 6 module
 * @param val value for the digit to display
 */
void iv6_set_digit(uint8_t const val) {
  s_segment_map map;
  map.all = 0;
  // set the lines properly
  // activate grid
  map.segment.segOUT4 = 1;
  // activate segments depending on the digit 
  if(digit_map[val] & DP) map.segment.segOUT2 = 1; 
  else map.segment.segOUT2 = 0;
  if(digit_map[val] & A) map.segment.segOUT0 = 1; 
  else map.segment.segOUT0 = 0;
  if(digit_map[val] & B) map.segment.segOUT1 = 1; 
  else map.segment.segOUT1 = 0;
  if(digit_map[val] & C) map.segment.segOUT3 = 1; 
  else map.segment.segOUT3 = 0;
  if(digit_map[val] & D) map.segment.segOUT16 = 1; 
  else map.segment.segOUT16 = 0;
  if(digit_map[val] & E) map.segment.segOUT17 = 1; 
  else map.segment.segOUT17 = 0;
  if(digit_map[val] & F) map.segment.segOUT18 = 1; 
  else map.segment.segOUT18 = 0;
  if(digit_map[val] & G) map.segment.segOUT19 = 1; 
  else map.segment.segOUT19 = 0;
  // write the data to the vfd controller	
  LXR_Tubeclock_Shield::vfd_controller_write(map);
  // load the data to the output latches
  LXR_Tubeclock_Shield::vfd_controller_load();
}


