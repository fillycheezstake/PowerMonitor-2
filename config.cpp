#include "config.h"
#include <Arduino.h>
#include <EEPROMex.h>  

//hardware limit - either 10 or 20 depending on if you have the expansion board.
#define MAX_NUM_CURRENT_CHANNELS 20

#define ADC0_AVG 8
#define ADC1_AVG 8
#define ADC0_RES 12
#define ADC1_RES 12

#define EEPROM_CURRENT_CALIB_SIZE       4  //a float is 4 bytes
#define EEPROM_VOLTAGE_CALIB_SIZE       4  //a float is 4 bytes
#define EEPROM_PHASE_CALIB_SIZE         4  //a float is 4 bytes
#define EEPROM_CHANNEL_NAME_SIZE        10 //a char is 1 byte, string must end in end byte
#define EEPROM_SIZE                     2048 //Teensy 3.2 has 2048 bytes of EEPROM 

#define EEPROM_CURRENT_CALIB_START        0
#define EEPROM_CURRENT_CALIB_END          (EEPROM_CURRENT_CALIB_START + EEPROM_CURRENT_CALIB_SIZE * MAX_NUM_CURRENT_CHANNELS)
#define EEPROM_VOLTAGE_CALIB_START        EEPROM_CURRENT_CALIB_END
#define EEPROM_VOLTAGE_CALIB_END          (EEPROM_VOLTAGE_CALIB_START + EEPROM_VOLTAGE_CALIB_SIZE * MAX_NUM_CURRENT_CHANNELS)
#define EEPROM_PHASE_CALIB_START          EEPROM_VOLTAGE_CALIB_END
#define EEPROM_PHASE_CALIB_END            (EEPROM_PHASE_CALIB_START + EEPROM_PHASE_CALIB_SIZE * MAX_NUM_CURRENT_CHANNELS)
#define EEPROM_CHANNEL_NAME_START         EEPROM_PHASE_CALIB_END
#define EEPROM_CHANNEL_NAME_END           (EEPROM_CHANNEL_NAME_START + EEPROM_CHANNEL_NAME_SIZE * MAX_NUM_CURRENT_CHANNELS)

float RealPower[NUM_CURRENT_CHANNELS] = {0};
float ApparentPower[NUM_CURRENT_CHANNELS] = {0};
float current[NUM_CURRENT_CHANNELS] = {0};
float PowerFactor[NUM_CURRENT_CHANNELS] = {0};
float voltage[NUM_CURRENT_CHANNELS] = {0};


// -------------------------------------------------------------------
// Reset EEPROM, wipes all settings
// -------------------------------------------------------------------
void ResetEEPROM(){
  //DEBUG.println("Erasing EEPROM");
  for (int i = 0; i < EEPROM_SIZE; ++i) {
    EEPROM.write(i, 0);
    //DEBUG.print("#");
  }
}

void EEPROM_read_string(int start, int count, String& val) {
  for (int i = 0; i < count; ++i){
    byte c = EEPROM.read(start+i);
    if (c!=0 && c!=255) val += (char) c;
  }
}

void EEPROM_write_string(int start, int count, String val) {
  for (int i = 0; i < count; ++i){
    if (i<(signed int)val.length()) {
      EEPROM.write(start+i, val[i]);
    } else {
      EEPROM.write(start+i, 0);
    }
  }
}



// -------------------------------------------------------------------
// Load saved settings from EEPROM
// -------------------------------------------------------------------

void config_load_current_calib(float test[])
{
  
  for (int i=0; i < NUM_CURRENT_CHANNELS; i++){ 
     test[i] = EEPROM.readFloat(EEPROM_CURRENT_CALIB_START + (4*i));
  }
}

void config_load_voltage_calib(float test[])
{
  for (int i=0; i < NUM_CURRENT_CHANNELS; i++){ 
     test[i] = EEPROM.readFloat(EEPROM_VOLTAGE_CALIB_START + (4*i));
  }
}
void config_load_phase_calib(float test[])
{
  for (int i=0; i < NUM_CURRENT_CHANNELS; i++){ 
     test[i] = EEPROM.readFloat(EEPROM_PHASE_CALIB_START + (4*i));
  }
}

void config_save_current_calib(float test[], int ctNum)
{
  
  if (ctNum == 99){
    for (int i=0; i < NUM_CURRENT_CHANNELS; i++){ 
       EEPROM.updateFloat(EEPROM_CURRENT_CALIB_START + i * EEPROM_CURRENT_CALIB_SIZE,test[i]);
    }       
  }  
  else {
      EEPROM.updateFloat(EEPROM_CURRENT_CALIB_START + ctNum * EEPROM_CURRENT_CALIB_SIZE,test[ctNum]);
  } 


}

void config_save_voltage_calib(float test[], int ctNum)
{  

  if (ctNum == 99){
    for (int i=0; i < NUM_CURRENT_CHANNELS; i++){ 
       EEPROM.updateFloat(EEPROM_VOLTAGE_CALIB_START + i * EEPROM_VOLTAGE_CALIB_SIZE,test[i]);
    }    
  }
  else {
    EEPROM.updateFloat(EEPROM_VOLTAGE_CALIB_START + ctNum * EEPROM_VOLTAGE_CALIB_SIZE,test[ctNum]);
  }

}

void config_save_phase_calib(float test[], int ctNum)
{
  if (ctNum == 99){
    for (int i=0; i < NUM_CURRENT_CHANNELS; i++){ 
       EEPROM.updateFloat(EEPROM_PHASE_CALIB_START + i * EEPROM_PHASE_CALIB_SIZE,test[i]);
    }    
  }
  else {
      EEPROM.updateFloat(EEPROM_PHASE_CALIB_START + ctNum * EEPROM_PHASE_CALIB_SIZE,test[ctNum]);
  }
}



void config_reset()
{
  ResetEEPROM();
}
