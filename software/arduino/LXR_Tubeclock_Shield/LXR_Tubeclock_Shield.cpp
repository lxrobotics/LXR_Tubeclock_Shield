/**
 * @author Alexander Entinger, MSc / LXRobotics
 * @brief this file implements the control interface for the LXRobotics Tubeclock Shield
 * @file LXR_Tubeclock_Shield.cpp
 * @license Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0) ( http://creativecommons.org/licenses/by-nc-sa/3.0/ ) 
 */

#include "LXR_Tubeclock_Shield.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* DEFINES */

// BOOST = D6 = PD6 = OC0A
#define BOOST_DDR	        (DDRD)
#define BOOST_PORT	        (PORTD)
#define BOOST		        (1<<6)
// VFD_POWER = A0 = ADC0 = PC0
#define VFD_POWER_DDR	        (DDRC)
#define VFD_POWER_PORT	        (PORTC)
#define VFD_POWER		(1<<0)
// BLANK = D9 = PB1
#define BLANK_DDR		(DDRB)
#define BLANK_PORT		(PORTB)
#define BLANK			(1<<1)
// LOAD = D8 = PB0
#define LOAD_DDR		(DDRB)
#define LOAD_PORT		(PORTB)
#define LOAD			(1<<0)
// SCK = D12 = PB5
#define SCK_DDR			(DDRB)
#define SCK			(1<<5)
// MOSI = D11 = PB3
#define MOSI_DDR		(DDRB)
#define MOSI			(1<<3)
// SS = D10 = PB2 // needs to be set to output to ensure safe spi operation
#define SS_DDR			(DDRB)
#define SS_PORT			(PORTB)
#define SS			(1<<2)
// T1 = D4 = PD4 = PCINT20
#define T1_DDR                  (DDRD)
#define T1_PORT                 (PORTD)
#define T1_PIN                  (PIND)
#define T1                      (1<<4)
// T2 = D5 = PD5 = PCINT21
#define T2_DDR                  (DDRD)
#define T2_PORT                 (PORTD)
#define T2_PIN                  (PIND)
#define T2                      (1<<5)

// filtered tube feedback voltage is connected with A1 = ADC1
#define TUBE_VOLTAGE_AS_ADC_INPUT \
ADMUX &= ~((1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (1<<MUX0)); \
	ADMUX |= (1<<MUX0); 
// LDR is connected with A2 = ADC2
#define LDR_VOLTAGE_AS_ADC_INPUT \
ADMUX &= ~((1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (1<<MUX0)); \
	ADMUX |= (1<<MUX1);
// NTC is connected with A3 = ADC3
#define NTC_VOLTAGE_AS_ADC_INPUT \
ADMUX &= ~((1<<MUX3) | (1<<MUX2) | (1<<MUX1)  | (1<<MUX0)); \
	ADMUX |= (1<<MUX1) | (1<<MUX0);

static volatile uint16_t m_tube_voltage = 0;
static volatile uint16_t m_ldr_voltage = 0;
static volatile uint16_t m_ntc_voltage = 0;
static volatile button_callback m_button_callback_func[2] = {
  0,0};

/* GLOBAL FUNCTIONS */

/** 
 * @brief initializes the tube clock shield module
 */
void LXR_Tubeclock_Shield::begin() {
  ADMUX = 0x00; 
  ADCSRA = 0x00; 
  ADCSRB = 0x00;
  // select AVCC as reference voltage, external capacitor at AREF pin
  ADMUX |= (1<<REFS0); 
  ADMUX &= ~(1<<REFS1);
  // disable digital input on ADC1 (tube) and ADC0 (LDR)
  //DIDR0 |= (1<<ADC0D) | (1<<ADC1D);
  // select adc input
  TUBE_VOLTAGE_AS_ADC_INPUT;	
  // enable all prescaler bits => prescaler = 128 => f_ADC = 125 kHz
  ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
  // enable auto trigger
  ADCSRA |= (1<<ADATE);
  // select auto trigger source : timer 0 overflow
  ADCSRB |= (1<<ADTS2);
  // enable the adc interrupt
  ADCSRA |= (1<<ADIE);
  // enable the adc
  ADCSRA |= (1<<ADEN);

  // set boost as output and to low
  BOOST_PORT &= ~BOOST;
  BOOST_DDR |= BOOST;

  // set timer 0 to phase correct pwm mode, timer mode 1, TOP = 0xFF
  TCCR0A = (1<<WGM00);
  // reset the timer value
  TCNT0 = 0;
  // reset TIMSK0 since its set by the great arduino ide
  TIMSK0 = 0x00;
  // set OCR0A
  OCR0A = 0;

  // VFD_POWER to output and high (power supply to maxim and v_fila cut off)
  VFD_POWER_PORT |= VFD_POWER;
  VFD_POWER_DDR |= VFD_POWER;

  // BLANK to output and high (all segments are blanked)
  BLANK_PORT |= BLANK;
  BLANK_DDR |= BLANK;

  // LOAD to output and low (latches hold their data)
  LOAD_PORT &= ~LOAD;
  LOAD_DDR |= LOAD;

  // init spi as master, max spi clk for MAX6921 according to datasheet = 5 MHz (Period = 200 ns)
  PRR &= ~(1<<PRSPI); // ensure that the spi power supply is turned on
  SCK_DDR |= SCK; 
  MOSI_DDR |= MOSI; 
  SS_DDR |= SS; 
  SS_PORT |= SS; // turn the spi pin direction to outputs
  SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0); // enable spi in master mode, fSPI = fOSC/16 = 1 MHz

  // select T1 and T2 as inputs withput pullups (external pullups available
  T1_PORT &= ~T1;
  T1_DDR &= ~T1;
  T2_PORT &= ~T2;
  T2_DDR &= ~T2;
  // enable pin change interrupts on PD4 (PCINT20) and PD5 (PCINT21)
  PCMSK2 |= (1<<PCINT20) | (1<<PCINT21);
  // enable pin change interrupt 
  PCICR |= (1<<PCIE2);

}

/** 
 * @brief returns the voltage at the ldr resistor as determined by the adc
 */
uint16_t LXR_Tubeclock_Shield::get_ldr_voltage() {
  return m_ldr_voltage;
}

/** 
 * @brief returns the voltage at the ntc resistor as determined by the adc
 */
uint16_t LXR_Tubeclock_Shield::get_ntc_voltage() {
  return m_ntc_voltage;
}

/**
 * @brief registers a callback function which is called when the selected button is pressed, the registered function runs in isr context and therefor needs to be short and under no circumstances blocking
 */
void LXR_Tubeclock_Shield::register_button_callback(E_BUTTON_SELECT const sel, button_callback cb) {
  m_button_callback_func[sel] = cb;
}

static uint8_t prev_t1 = 1;
static uint8_t prev_t2 = 1;
/**
 * @brief interrupt service handler for pressed buttons
 */
ISR(PCINT2_vect) {
  uint8_t const t1_now = ((T1_PIN & T1) > 0) ? 1 : 0;
  uint8_t const t2_now = ((T2_PIN & T2) > 0) ? 1 : 0;
  if(prev_t1 == 1 && t1_now == 0) { // button 1 pressed
    if(m_button_callback_func[BUTTON1] != 0) m_button_callback_func[BUTTON1]();
  }
  if(prev_t2 == 1 && t2_now == 0) { // button 2 pressed
    if(m_button_callback_func[BUTTON2] != 0) m_button_callback_func[BUTTON2]();
  }
  prev_t1 = t1_now;
  prev_t2 = t2_now;
}

/** 
 * @brief sets the output voltage of the step-up-converter
 * @param volt voltage to be set
 */
void LXR_Tubeclock_Shield::set_tube_voltage(uint8_t const volt) {
  // adc input voltage divider divides by 14 - max voltage 70 Volt -> 5 V at ADC Input
  // weve got 10 bit precision, therefor 1024 Steps, 70 V => 1024, 35 V = > 512, 0 V => 0
  if(volt <= MAX_TUBE_VOLTAGE) {
    uint32_t const multiplied_voltage = (uint32_t)(volt) << 10; // multiplication by 1024
    m_tube_voltage = (uint16_t)(multiplied_voltage / (uint32_t)(MAX_TUBE_VOLTAGE+1)); // division by 70 results in the 10 bit value representing the desired voltage on the adc pin
  }	
}

/** 
 * @brief enables the tube voltage
 */
void LXR_Tubeclock_Shield::enable_tube_voltage() {
  // reset ocr0a
  OCR0A = 0;
  // clear on compare match when counting up, set on compare match when counting down
  TCCR0A &= ~(1<<COM0A0); 
  TCCR0A |= (1<<COM0A1);
  // activate timer with prescaler 1 => f_PWM = 62.5 kHz
  TCCR0B = (1<<CS01);
}

/** 
 * @brief disables the tube voltage
 */
void LXR_Tubeclock_Shield::disable_tube_voltage() {
  // deactivate the timer	
  TCCR0B &= ~((1<<CS02) | (1<<CS01) | (1<CS00));
  // disconnect OC0A from timer logic
  TCCR0A &= ~((1<<COM0A0) | (1<<COM0A1));
  // set OC0A output to low
  BOOST_PORT &= ~BOOST;
}

typedef enum {
  E_TUBE_VOLTAGE = 0, E_LDR = 1, E_NTC = 2} 
E_ADC_MEASUREMENT;
static volatile E_ADC_MEASUREMENT adc_meas = E_TUBE_VOLTAGE;
/** 
 * @brief adc interrupt service routine - called when a conversion is complete
 */
ISR(ADC_vect) {
  static uint8_t state_change_counter = 0;
  // clear the timer 0 overflow interrupt flag, since no isr is executed in which this would be handled automatically
  TIFR0 |= (1<<TOV0);
  // the output voltage control feedback loop
  uint16_t const adc_low = (uint16_t)(ADCL);
  uint16_t const adc_high = (uint16_t)(ADCH) << 8;
  if(adc_meas == E_TUBE_VOLTAGE) {
    uint16_t const current_tube_voltage = adc_high | adc_low;
    if(current_tube_voltage > m_tube_voltage) { 
      if(OCR0A > 1) OCR0A = OCR0A - 1; 
    }
    else if(current_tube_voltage < m_tube_voltage)	{ 
      if(OCR0A < 255) OCR0A = OCR0A + 1; 
    }
    state_change_counter++;
    if(state_change_counter >= 200) {
      state_change_counter = 0;
      adc_meas = E_LDR;
      LDR_VOLTAGE_AS_ADC_INPUT;
    }
  } 
  else if(adc_meas == E_LDR) {
    m_ldr_voltage = adc_high | adc_low;
    adc_meas = E_NTC;
    NTC_VOLTAGE_AS_ADC_INPUT;
  } 
  else if(adc_meas == E_NTC) {
    m_ntc_voltage = adc_high | adc_low;
    adc_meas = E_TUBE_VOLTAGE;
    TUBE_VOLTAGE_AS_ADC_INPUT;
  }		
}

/**
 * @brief enables the power supply to the vfd controller and v_fila
 */
void LXR_Tubeclock_Shield::enable_vfd_controller() {
  VFD_POWER_PORT &= ~VFD_POWER;
}

/**
 * @brief disables the power supply to the vfd controller and v_fila
 */
void LXR_Tubeclock_Shield::disable_vfd_controller() {
  VFD_POWER_PORT |= VFD_POWER;
}

/**
 * @brief turns all segments off
 */
void LXR_Tubeclock_Shield::vfd_controller_blank_on() {
  BLANK_PORT |= BLANK;
}

/** 
 * @brief turns all segments on
 */
void LXR_Tubeclock_Shield::vfd_controller_blank_off() {
  BLANK_PORT &= ~BLANK;
}

/** 
 * @brief writes the segment map to the vfd controller
 */
void LXR_Tubeclock_Shield::vfd_controller_write(s_segment_map const segs) {
  uint8_t byte[3] = {
    0    };
  byte[0] =	0x00 | (segs.segment.segOUT19 << 3) | (segs.segment.segOUT18 << 2) | (segs.segment.segOUT17 << 1) | (segs.segment.segOUT16 << 0);
  byte[1] =	(segs.segment.segOUT15 << 7) | (segs.segment.segOUT14 << 6) | (segs.segment.segOUT13 << 5) | (segs.segment.segOUT12 << 4) |
    (segs.segment.segOUT11 << 3) | (segs.segment.segOUT10 << 2) | (segs.segment.segOUT9 << 1) | (segs.segment.segOUT8 << 0);
  byte[2] =	(segs.segment.segOUT7 << 7) | (segs.segment.segOUT6 << 6) | (segs.segment.segOUT5 << 5) | (segs.segment.segOUT4 << 4) |
    (segs.segment.segOUT3 << 3) | (segs.segment.segOUT2 << 2) | (segs.segment.segOUT1 << 1) | (segs.segment.segOUT0 << 0);

  SPDR = byte[0];
  while (!(SPSR & (1<<SPIF)));
  SPDR = byte[1];
  while (!(SPSR & (1<<SPIF)));
  SPDR = byte[2];
  while (!(SPSR & (1<<SPIF)));
}

/**
 * @brief load the shifted data into the latches
 */
void LXR_Tubeclock_Shield::vfd_controller_load() {
  LOAD_PORT |= LOAD;
  _delay_ms(1);
  LOAD_PORT &= ~LOAD;
}


