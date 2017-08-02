//////////////////////////////////////////
// Peltier Control Software for PCR
// Uses a Dallas DS18B20 Temperature Sensor (could be replaced by any other type)
// And a VNH3SP30 Motor Driver for Peltier control
//
// By Felix Bonowski (2017)
//
// Licensed as Creative Commons Attribution-ShareAlike 3.0 (CC BY-SA 3.0) 
//
/////////////////////////////////////////////////////////////////////////


#pragma once
#include "Arduino.h"
//#include "debugPrint.h"

class TemperaturePidController {
  public:
    TemperaturePidController(
      float cP, //  outputUnits/inputUnits
      float cI, //  outputUnits/inputUnits/second
      float cD ,
      int peltierAPin,
      int peltierBPin,
      int peltierPwmPin,
      float targetTemperature);
    void setup();
    void enable();
    void disable();
    void setTarget(float neTargetValue);
    void control(float currentValue);
    float cP; //  outputUnits/inputUnits
    float cI; //  outputUnits/inputUnits/second
    float cD = 0;

    int peltierAPin; // these determine direction of current flow and thus heating/cooling
    int peltierBPin;

    int peltierPwmPin;

    float lastOutVal;
    float getOutput(float input, float minOut, float maxOut);
    float targetValue;
    bool controlEnabled = false;

    unsigned long lastMillis;
    float integratedOutput; //outputUnits
};

TemperaturePidController::TemperaturePidController(
  float cP, //  outputUnits/inputUnits
  float cI, //  outputUnits/inputUnits/second
  float cD ,
  int peltierAPin,
  int peltierBPin,
  int peltierPwmPin,
  float targetTemperature):
  cP(cP),
  cI(cI),
  cD(cD),
  peltierAPin(peltierAPin),
  peltierBPin(peltierBPin),
  peltierPwmPin(peltierPwmPin),
  targetValue(targetTemperature)
{
  targetValue = 25;
  integratedOutput = 0;
  lastMillis = millis();
};
void TemperaturePidController::setup() {
  pinMode(peltierAPin, OUTPUT);
  pinMode(peltierBPin, OUTPUT);

  pinMode(peltierPwmPin, OUTPUT);
  analogWrite(peltierPwmPin, 0);
  lastMillis = millis();
}
void TemperaturePidController::setTarget(float newTargetValue) {
  targetValue = constrain(newTargetValue, MIN_TEMP, MAX_TEMP);
};
void TemperaturePidController::enable() {
  lastMillis = millis();
  controlEnabled = true;
}
void TemperaturePidController::disable() {
  analogWrite(peltierPwmPin, 0);
  controlEnabled = false;
  integratedOutput = 0;
  lastOutVal = 0;
}


void TemperaturePidController::control(float currentValue) {
  if ( controlEnabled) {
    lastOutVal = getOutput(currentValue, -255, 255);
  } else {
    lastOutVal = 0;
  }
  //set Heat/Cool by current direction
  if (lastOutVal > 0) {
    digitalWrite(peltierAPin, HIGH);
    digitalWrite(peltierBPin, LOW);
  } else {
    digitalWrite(peltierAPin, LOW);
    digitalWrite(peltierBPin, HIGH);
  }

  int pwmVal = abs(lastOutVal);
  analogWrite(peltierPwmPin, pwmVal);

}
float TemperaturePidController::getOutput(float input, float minOut, float maxOut) {
  float diff = targetValue - input;
  float out = diff * cP + integratedOutput;
  //  TRACE("PID:");
  //  TRACELN(diff);
  //  TRACELN(out);
  //to prevent integral windup, integrate only when output isn't saturated
  float deltaT = ((float)(millis() - lastMillis)) / 1000.0;
  float deltaOutput = diff * cI;
  lastMillis = millis();
  if ((out < maxOut || deltaOutput < 0) && (out > minOut || deltaOutput > 0))integratedOutput += deltaOutput * deltaT;

  //constrain output to legal range
  if (out < minOut)out = minOut;
  if (out > maxOut)out = maxOut;
  return (out);
};


