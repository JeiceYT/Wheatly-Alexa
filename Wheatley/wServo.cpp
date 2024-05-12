/*
 * Wheatley Servo Class - Source
 *   by Steve Turner (arduinokiln@gmail.com)
 */

#include "Arduino.h"
#include "wServo.h"

  //****************************************************************************************************************************
 //  Constructor
//******************************************************************************************************************************
wServo::wServo(byte pin, int minPos, int homePos, int maxPos, byte steps) {

  _homePos = homePos;
  _maxPos = maxPos;
  _minPos = minPos;
  _pin = pin;
  _steps = steps;
  _updateInterval = 2500;
    
}

  //****************************************************************************************************************************
 //  ATTACH
//******************************************************************************************************************************
void wServo::attach() {

  _atNewPos = true;
  _currentPos = _homePos;
  _newPos = _homePos;

  _servo.attach(_pin);
  
  // Set the seed for random # generator
  randomSeed(micros());
  
}

  //****************************************************************************************************************************
 //  ATNEWPOS: Is servo at new position?
//******************************************************************************************************************************
bool wServo::atNewPos() {

  return _atNewPos;
  
}

  //****************************************************************************************************************************
 //  DETACH
//******************************************************************************************************************************
void wServo::detach() {

  _atNewPos = false;
  _servo.detach();
  
}

  //****************************************************************************************************************************
 //  MOVEHOME: Set new position to home
//******************************************************************************************************************************
void wServo::moveHome() {

  _newPos = _homePos;
  _setDir();
  _atNewPos = false;
  
}

  //****************************************************************************************************************************
 //  MOVEMAX: Set new position to max
//******************************************************************************************************************************
void wServo::moveMax() {

  _newPos = _maxPos;
  _setDir();
  _atNewPos = false;
  
}

  //****************************************************************************************************************************
 //  MOVEMIN: Set new position to min
//******************************************************************************************************************************
void wServo::moveMin() {

  _newPos = _minPos;
  _setDir();
  _atNewPos = false;
  
}

  //****************************************************************************************************************************
 //  MOVERANDOM: Set new random position
//******************************************************************************************************************************
void wServo::moveRandom() {

  _newPos = random(_minPos, _maxPos + 1);
  _setDir();
  _atNewPos = false;

}

  //****************************************************************************************************************************
 //  _SETDIR: Set direction to move
//******************************************************************************************************************************
void wServo::_setDir() {

  if (_newPos >= _currentPos) {
    _dir = +1;
  }
  else {
    _dir = -1;
  }
  
}

  //****************************************************************************************************************************
 //  UPDATE: Control for servo.  Based on timer, no delays or interrupts
//******************************************************************************************************************************
void wServo::update() {

  // Check if you are already in position
  if (_currentPos == _newPos) {
    _atNewPos = true;
    return;
  }

  unsigned long _currentTime = micros();

  if (_currentTime - _lastUpdate > _updateInterval) {

    // Figure out the next position to take in the step
    _currentPos += (_dir * _steps);
    if ((_currentPos - _newPos) * _dir > 0 ) {  // Don't overtravel
      _currentPos = _newPos;
    }

    // Move the stepper to the next step position
    _servo.writeMicroseconds(_currentPos);

    // Remember when you moved
    _lastUpdate = _currentTime;
    
  }
  
}
