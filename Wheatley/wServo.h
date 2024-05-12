/*
 * Wheatley Servo Class - Header
 *   by Steve Turner (arduinokiln@gmail.com)
 */

#ifndef wServo_h
#define wServo_h

#include "Arduino.h"
#include <Servo.h>

class wServo {
  
  public:
    wServo(byte pin, int minPos, int homePos, int maxPos, byte steps);  // Constructor
    bool atNewPos();
    void attach();
    void detach();
    void moveHome();
    void moveMax();
    void moveMin();
    void moveRandom();
    void update();

  private:
    bool _atNewPos;             // Are you at the new position?
    int _currentPos;            // Current position (microseconds)
    int _dir;                   // Direction to move (+1 forward, -1 backwards)
    int _homePos;               // Home servo position (microseconds)
    unsigned long _lastUpdate;  // Time of last update (microseconds since power on)
    int _maxPos;                // Maximum servo position (microseconds)
    int _minPos;                // Minimum servo position (microseconds)
    int _newPos;                // New position to move to (microseconds)
    byte _pin;                  // Pin # assigned to servo
    void _setDir();             // Set direction to move
    Servo _servo;               // Servo, duh
    byte _steps;                // Number of additional microsecond "steps" to take on each update (ie speed)
    int _updateInterval;        // How often to update servo position (microseconds)
  
};

#endif
