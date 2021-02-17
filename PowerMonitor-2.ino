//
//Power Monitor V2 - Philip Ahlers
//
//Uses a Modified Emonlib - https://github.com/openenergymonitor/EmonLib
//
//The idea is to get data from multiple CTs and voltage waveform sources to get accurate voltage, power factor, real power, and apparent power.
//Then it sends data via serial - key:value pairs (similar to json) - to a receiving device - an ESP8266, for sending to a database of some sort.
//
//Tested with Arduino 1.8.13 and Teensyduino 1.53

#include "config.h"
#include "EmonLib.h"
#include <ArduinoJson.h>



EnergyMonitor CT[NUM_CURRENT_CHANNELS];

//float voltageCalib[NUM_CURRENT_CHANNELS] = {124,124,124,124,124,124,124,124,124,124,124,124,124,124,124,124,124,124,124,124};
//float currentCalib[NUM_CURRENT_CHANNELS] = {15,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16};
//float phaseCalib[NUM_CURRENT_CHANNELS] = {1.7,1.7,1.7,1.7,1.7,1.7,1.7,1.7,1.7,1.7,1.7,1.7,1.7,1.7,1.7,1.7,1.7,1.7,1.7,1.7};

float voltageCalib[NUM_CURRENT_CHANNELS] = {};
float currentCalib[NUM_CURRENT_CHANNELS] = {};
float phaseCalib[NUM_CURRENT_CHANNELS] = {};

boolean tableMode = false;
String tableType = "a c";
int tableNum = 0;

unsigned long previousPushMillis = 0;

ADC *adc = new ADC();

void setup() {
    
  //USB serial
  Serial.begin(115200);

  //Serial connection to ESP8266
  Serial1.begin(115200);

// config_save_voltage_calib(voltageCalib);
//  config_save_phase_calib(phaseCalib);
//  config_save_current_calib(currentCalib);

  //loads the configuration values stored in EEPROM
  config_load_voltage_calib(voltageCalib);
  config_load_phase_calib(phaseCalib);
  config_load_current_calib(currentCalib);
  
  //just to let USB serial catch up
  delay(1000);

  //uses the Texas Instruments REF2033 as the VREF.
  adc->adc0->setReference(ADC_REFERENCE::REF_EXT);
  adc->adc1->setReference(ADC_REFERENCE::REF_EXT);
  
  adc->adc0->setAveraging(16);
  adc->adc1->setAveraging(16);
  
  adc->adc0->setResolution(12);
  adc->adc1->setResolution(12);

  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::MED_SPEED);
  adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::MED_SPEED);
  
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::MED_SPEED);
  adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::MED_SPEED);
  
  init_Emon();
    
}


void loop() {    
       
  unsigned long currentMillis = millis();
  String data;
  
  // If data received on EmonEsp serial
  if (Serial1.available()) {
    data = Serial1.readStringUntil('\n');
  }
  
  data.trim();
    
  //example: @@{"CT": 1, "cType": 1, "val": 1.2}
  if (data.startsWith("@@"))  {

    Serial.println("Request to set Calibration Values Received!");
    data = data.substring(2);
    
    StaticJsonDocument<200> jsonDocToRead;
    deserializeJson(jsonDocToRead, data);
    
    int CTNumber = jsonDocToRead["CT"];
    int cType    = jsonDocToRead["cType"];
    float val    = jsonDocToRead["val"];
    
    switch (cType) {
      case 1: // current calibration
      {
        currentCalib[CTNumber-1] = val;
        config_save_current_calib(currentCalib,CTNumber - 1);
        Serial1.print("Current calibration saved to EEPROM for CT number ");
        Serial1.println(CTNumber);
        init_Emon();
      }
      break;
      
      case 2: // phase calibration
      { 
        phaseCalib[CTNumber-1] = val;
        config_save_phase_calib(phaseCalib,CTNumber - 1);
        Serial1.println("Phase calibration saved to EEPROM for CT number ");
        Serial1.println(CTNumber);
        init_Emon();
      }
      break;
      
      case 3: // voltage calibration
      {
        voltageCalib[CTNumber-1] = val;
        config_save_voltage_calib(voltageCalib,CTNumber - 1);
        Serial1.println("Voltage calibration saved to EEPROM for CT number ");
        Serial1.println(CTNumber);
        init_Emon();
      }
      break;
      
      case 4: // all channels voltage calibration
      {
        for (int i=0; i < NUM_CURRENT_CHANNELS; i++) {
          voltageCalib[i] = val;
        }
        
        config_save_voltage_calib(voltageCalib,99);
        Serial1.println("All CTs voltage calibrations saved to EEPROM.");
        init_Emon();
      }
      break;
    
      default: 
      {
        Serial1.println("Error: Not a Valid Calibration Type");
      }  
      break;
      }
  }
//-------tablemode--------   
  else if (data.startsWith("table")) {
    tableMode = true;
    data = data.substring(5);
    data.trim();
    if (data == "a"){
      tableType = data;
    }
    else if (data == "a c"){
      tableType = data;
    }
    else if (data == "a nc"){
      tableType = data;
    }
    else if (data.toInt() <= 20 and data.toInt() >= 0){
      tableNum = data.toInt() - 1;
      tableType = "";      
    }

    else {
        tableNum = 0;
    }
    Serial1.println("Table mode - sending to ESP in table format");

//------quit tablemode-------
  } else if (data.startsWith("quit table")){
    tableMode = false;
    Serial1.println("Normal mode - sending to ESP in key:value format");
  }

//--------helpmode----------  
  else if (data.startsWith("help")) {
    Serial1.println("Type 'table' follwed by a channel number to print data in a more human-readable format, and 'quit table' to stop.");
    Serial1.println("Type 'table' followed by 'a' for all channels and all data. Type 'table' followed by 'a c' for all channels but only calibration data. Type 'table' followed by 'a nc' for all channels but no calibration data.");
    Serial1.println("Type \"@@{\"CT\": <CT NUMBER>, \"cType\": <CALIBRATION TYPE>, \"val\": <CALIBRATION>}\" To calibrate a channel.");
    Serial1.println("<CT NUMBER> is the CT channel you wish to calibrate.");
    Serial1.println("<CALIBRATION TYPE> is as follows - 1 for current, 2 for phase, 3 for individual channel voltage, and 4 for all channels voltage. Note option 4 ignores the <CT NUMBER> field.");
    Serial1.println("<VALUE> is a unitless calibration constant.");
  }
  
  //calculate the current power - calcVI(num_crosses, timeout)
  //to find each zero crossing is the 1/2 the period  - so for 60hz: (1/60) / 2 = .00833 seconds per cross. A 20 cross sample will take .1666 seconds
  //so if you use 20 zero crossings, reading 10 CTs will take ~1.66 seconds, reading 20 CTs will take ~3.33 seconds. 
  // this implementation: 19 CTs with 10 crosses should take around 1.583 seconds.  Tested at ~1.650 seconds.  The discrepancy is due to the fact that the code waits for a zero crossing.

  for (int i=0; i < NUM_CURRENT_CHANNELS; i++) {
    CT[i].calcVI(10,50, adc);
    RealPower[i] = CT[i].realPower;
    ApparentPower[i] = CT[i].apparentPower;
    current[i] = CT[i].Irms;
    PowerFactor[i] = CT[i].powerFactor;
    voltage[i] = CT[i].Vrms;
  }

  //every push_freq seconds, push data to ESP, either in table form or for EmonESP
  if (currentMillis - previousPushMillis > push_freq) {

  String StringToSend = "";
  
  if (tableMode == false)  {
      for (int i = 0; i < NUM_CURRENT_CHANNELS; i++) {
         StringToSend += "CT" + (String)(i+1) + "_Rp:" + (String) RealPower[i] + ",";
      }
      StringToSend += "voltage:" + (String) voltage[0];
      Serial1.println(StringToSend);
      Serial.println("Sent Data to ESP");
  }
  else {
 

    if (tableType == "a"){
      Serial1.println("CT   |   Voltage   | Real Power |   Apparent Power  | Power Factor |    Current    | Current Calibration | Phase Calibration | Voltage Calibration");
      for (int i=0; i < NUM_CURRENT_CHANNELS; i++) {
        Serial1.println((String)(i+1) + "\t" + (String) voltage[i] + "\t\t" + (String) RealPower[i] + " \t\t" + (String) ApparentPower[i] + "\t\t" + (String) PowerFactor[i] + "\t\t" + (String) current[i] + " \t\t" + (String) currentCalib[i] + "\t\t\t" + (String) phaseCalib[i] + "\t\t\t" + (String) voltageCalib[i]); 
      }
    }
    
    else if (tableType == "a c"){
      Serial1.println("CT  | Current Calibration | Phase Calibration | Voltage Calibration");
      for (int i=0; i < NUM_CURRENT_CHANNELS; i++) {
        Serial1.println((String)(i+1) + " \t\t" + (String) currentCalib[i] + "\t\t" + (String) phaseCalib[i] + "\t\t" + (String) voltageCalib[i]);
      }
    }  
    else if (tableType == "a nc"){
      Serial1.println("CT   |   Voltage   | Real Power |   Apparent Power  | Power Factor |    Current");
      for (int i=0; i < NUM_CURRENT_CHANNELS; i++) {
        Serial1.println((String)(i+1) + "\t" + (String) voltage[i] + "\t\t" + (String) RealPower[i] + " \t\t" + (String) ApparentPower[i] + "\t\t" + (String) PowerFactor[i] + "\t\t" + (String) current[i]); 
      }
    }
    
    else {
      int i = tableNum;
      Serial1.println("CT   |  Voltage  | Real Power |   Apparent Power  | Power Factor |    Current    | Current Calibration | Phase Calibration | Voltage Calibration");
      Serial1.println((String)(i+1) + "\t" + (String) voltage[i] + "\t\t" + (String) RealPower[i] + " \t\t" + (String) ApparentPower[i] + "\t\t" + (String) PowerFactor[i] + "\t\t" + (String) current[i] + " \t\t" + (String) currentCalib[i] + "\t\t\t" + (String) phaseCalib[i] + "\t\t\t" + (String) voltageCalib[i]); 
    }


  }
    
    
    
    previousPushMillis = millis();
  
  }  //end Pushing data to ESP
    
} //end Loop()



//------------------------------------------------------------
void init_Emon(){

  config_load_current_calib(currentCalib);
  config_load_voltage_calib(voltageCalib);
  config_load_phase_calib(phaseCalib);
  Serial.println("Loaded current, voltage, and phase calibrations from EEPROM into Arrays.");

  //voltage calibration:
  //voltage(input_pin, volt_scaling_const, phase_shift)
  for (int i=0; i < NUM_CURRENT_CHANNELS; i++) {
    CT[i].voltage(VOLTAGE_CHANNEL_PINS[i], voltageCalib[i], phaseCalib[i]);
  }
  Serial.println("Loaded voltage & phase calibrations from arrays into EmonLib Objects.");

  //current calibration:
  //current(input_pin, i_scaling_const)
  for (int i=0; i < NUM_CURRENT_CHANNELS; i++) {
    CT[i].current(CURRENT_CHANNEL_PINS[i], currentCalib[i]);
  }
  Serial.println("Loaded current calibrations from arrays into EmonLib Objects.");
    
}
