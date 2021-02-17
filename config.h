#ifndef config_h
#define config_h

#include <Arduino.h>

#define NUM_CURRENT_CHANNELS 19
#define NUM_VOLTAGE_CHANNELS 1

#define push_freq 3000

const int CURRENT_CHANNEL_PINS[] =  {A0,A1,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,A16,A17,A18,A19,A20};
const int VOLTAGE_CHANNEL_PINS[] = {A2,A2,A2,A2,A2,A2,A2,A2,A2,A2,A2,A2,A2,A2,A2,A2,A2,A2,A2,A2};

// -------------------------------------------------------------------
// Load and save the Teensy config. This stores and gets CT calibration values.
//
// This initial implementation saves the config to the EEPROM area of flash
// -------------------------------------------------------------------

// Global config varables

//These all store the latest calculated data
extern float RealPower[];
extern float ApparentPower[];
extern float current[];
extern float PowerFactor[];
extern float voltage[];

extern void config_load_current_calib(float test[]);
extern void config_load_voltage_calib(float test[]);
extern void config_load_phase_calib(float test[]);
extern void config_save_current_calib(float test[], int ctNum);
extern void config_save_voltage_calib(float test[], int ctNum);
extern void config_save_phase_calib(float test[], int ctNum);
extern void config_reset();

#endif
