/**
 * @author Alexander Entinger, MSc / LXRobotics
 * @brief this module implements the multiplexed control of the digits of the VFD IV-18 tube
 * @file LXR_Tubeclock_Shield_IV18.h
 * @license Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0) ( http://creativecommons.org/licenses/by-nc-sa/3.0/ )
 */

#ifndef LXR_TUBECLOCK_SHIELD_IV18_H_
#define LXR_TUBECLOCK_SHIELD_IV18_H_

#include <stdint.h>

#define DISPLAY_SIZE	(8)

class LXR_Tubeclock_Shield_IV18 {
public:
  /**
   * @brief this function initializes the iv 18 library
   */
  static void begin();
  /**
   * @brief writes a string to the display, user has to ensure that the string is 8 byte long
   * @param data pointer to that string
   */
  static void write_string(uint8_t const * const data);

private:
  /**
   * @brief Constructor
   */
  LXR_Tubeclock_Shield_IV18() { 
  } 
};

#endif

