//////////////////////////////////////////
// Peltier Control Software for Arduino - written for use in Thermocyclers. By Felix Bonowski @ Bauhaus Weimar (2017)
//
// !Use only under constant supervision - your device _WILL_ cause a fire if it malfunctions!
//
// Needs OneWire (By Paul Stoffregen) and  "DallasTemperature" DS18B20 Libraries By (Miles Burton) (Download via Skech > Import library > Contributed)
// 
// Compatiple Sensors and Drivers:
// - Dallas DS18B20 Temperature Sensor (could be replaced by any other type. Good thermal coupling to via-holder is important!)
// - VNH3SP30 Motor Driver for Peltier control (cheap and high current, place in airflow of the peltier-fan if it overheats)
//
// Serial commands:
// - set temperature to ###.### (enables peltier):  "t###.###\n"
// - disable peltier:  "x\n"
//
// If the system heats when it should cool(and vice versa), reverse peltier polarity
//
// Licensed as Creative Commons Attribution-ShareAlike 3.0 (CC BY-SA 3.0) 
//
/////////////////////////////////////////////////////////////////////////

// Infos about the temp. sensor can be found here:https://www.milesburton.com/Dallas_Temperature_Control_Library
#include "OneWire_.h"
#include "DallasTemperature_.h"

#define MIN_TEMP 0
#define MAX_TEMP 95

#define ONE_WIRE_BUS 2  // Data wire is plugged into pin 2 on the Arduino
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.


// set up the controller for the Peltier Element
#include "temperaturePid.h"
TemperaturePidController controller(
  80,//float cP, //  outputUnits/inputUnits (found by experimentation)
  5,//float cI, //  outputUnits/inputUnits/second (found by experimentation)
  0,//float cD , //not implemented
  5,//int peltierAPin,
  4,//int peltierBPin,
  3,//int peltierPwmPin,
  15//float targetTemperature
);



void setup() {
  Serial.begin(115200);
  controller.setup();
  sensors.begin(); // IC Default 9 bit temp. res.
  sensors.setResolution(11);
}

void loop() {
  //retrieve commands from computer
  if (Serial.available() > 0) { // If there is Data
    int inByte = Serial.read();
    switch (inByte) {
      case 'x':                   // 'switch off regulation'
        {
          controller.disable();
          break;                    // höre hier auf.
        }

      case 't':                   // 'set temperature'
        {
          float inValue = Serial.parseFloat();
          controller.enable();
          controller.setTarget(inValue);
          Serial.print("Target Temperature set to ");
          Serial.println(inValue);
          break;                    // höre hier auf.
        }
      default:
        break;
    }
  }
  //read temperature
  sensors.requestTemperatures(); // Send the command to get temperatures

  float currentTemp = sensors.getTempCByIndex(0);
 //disconnected sensors report -127:
  if(currentTemp ==-127){
    Serial.println("Check Temp. Sensor connection!\n");
    controller.disable();
  }
  //check if the whole thing gets too hot and prevent it from catching fire:
  if(currentTemp >100||currentTemp <-20){
    Serial.println("Overheated - shutting down to prevent fire! (check polarity of peltier and driver\n");
    controller.disable();
  }
  
  controller.control(currentTemp);

  //debug out:
  Serial.print("Setpoint (°C):\t"); Serial.print(controller.targetValue); Serial.print("\t");
  Serial.print("Measured (°C):\t"); Serial.print(currentTemp); Serial.print("\t");
  Serial.print("Pwm (0-255):\t"); Serial.print(controller.lastOutVal); Serial.print("\t");
  Serial.print("time (ms):\t"); Serial.print(millis()); Serial.print("\n");
}
