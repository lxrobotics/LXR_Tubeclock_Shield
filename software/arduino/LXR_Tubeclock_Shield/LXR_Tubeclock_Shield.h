/**
 * @author Alexander Entinger, MSc / LXRobotics
 * @brief this file implements the control interface for the LXRobotics Tubeclock Shield
 * @file LXR_Tubeclock_Shield.h
 * @license Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0) ( http://creativecommons.org/licenses/by-nc-sa/3.0/ )
 */

#ifndef LXR_TUBECLOCK_SHIELD_H_
#define LXR_TUBECLOCK_SHIELD_H_

#include <stdint.h>

static uint8_t const MAX_TUBE_VOLTAGE = 69;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"

typedef union {
  struct {					
uint8_t segOUT0 : 
    1;
uint8_t segOUT1 : 
    1;
uint8_t segOUT2 : 
    1;
uint8_t segOUT3 : 
    1;
uint8_t segOUT4 : 
    1;
uint8_t segOUT5 : 
    1;
uint8_t segOUT6 : 
    1;
uint8_t segOUT7 : 
    1;
uint8_t segOUT8 : 
    1;
uint8_t segOUT9  : 
    1;
uint8_t segOUT10  : 
    1;
uint8_t segOUT11 : 
    1;
uint8_t segOUT12 : 
    1;
uint8_t segOUT13 : 
    1;
uint8_t segOUT14 : 
    1;
uint8_t segOUT15 : 
    1;
uint8_t segOUT16 : 
    1;
uint8_t segOUT17 : 
    1;
uint8_t segOUT18 : 
    1;
uint8_t segOUT19 : 
    1;
  } 
  segment;
  uint32_t all;            // Shortcut to all segments
} 
s_segment_map;

#pragma GCC diagnostic push

typedef void(*button_callback)(void); // typedef for registering callbacks when a button was pressed
typedef enum {BUTTON1 = 0, BUTTON2 = 1} E_BUTTON_SELECT;

class LXR_Tubeclock_Shield {
public:
  /**
   * @brief initialize the tubeclock shield
   */
  static void begin();

  /** 
   * @brief returns the voltage at the ldr resistor as determined by the adc
   */
  static uint16_t get_ldr_voltage();

  /** 
   * @brief returns the voltage at the ntc resistor as determined by the adc
   */
  static uint16_t get_ntc_voltage();
  
  /**
   * @brief registers a callback function which is called when the selected button is pressed, the registered function runs in isr context and therefor needs to be short and under no circumstances blocking
   */
  static void register_button_callback(E_BUTTON_SELECT const sel, button_callback cb);

  /** 
   * @brief sets the output voltage of the step-up-converter
   * @param volt voltage to be set
   */
  static void set_tube_voltage(uint8_t const volt);

  /** 
   * @brief enables the tube voltage
   */
  static void enable_tube_voltage();

  /** 
   * @brief disables the tube voltage
   */
  static void disable_tube_voltage();

  /**
   * @brief enables the power supply to the vfd controller and v_fila
   */
  static void enable_vfd_controller();

  /**
   * @brief disables the power supply to the vfd controller and v_fila
   */
  static void disable_vfd_controller();

  /**
   * @brief turns all segments off
   */
  static void vfd_controller_blank_on();

  /** 
   * @brief turns all segments on
   */
  static void vfd_controller_blank_off();

  /** 
   * @brief writes 4 byte to vfd controller (the last 20 bytes are relevant, the rest gets shifted out)
   */
  static void vfd_controller_write(s_segment_map const segs);

  /**
   * @brief load the shifted data into the latches
   */
  static void vfd_controller_load();
  
private:
  /** 
   * @brief Constructor
   */
  LXR_Tubeclock_Shield() { }
};

#endif




