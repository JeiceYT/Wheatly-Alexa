/*
 * Wheatley Personal Assistant: From the video game Portal 2.  Featuring the Amazon Echo Dot v3.
 *   by Steve Turner (arduinokiln@gmail.com)
 */

// Include all libraries
#include <Adafruit_NeoPixel.h>    // LED light control
#include <DFRobotDFPlayerMini.h>  // MP3 player
#include "thingProperties.h"      // Arduino IoT cloud
#include <wiring_private.h>       // Needed for SERCOM (extra UART serial port)
#include "wServo.h"               // Class for Wheatley servos

// Setup hardware pins (CHANGE THESE TO MATCH YOUR SETUP)
//                        0;  // MP3 serial transmit pin
//                        1;  // MP3 serial receive pin
const byte HORSERVO =     2;  // Horizontal servo pin
const byte LEDPIN =       4;  // LED data pin
const byte BTRXPIN =      5;  // Bluetooth serial receive pin.  DON'T CHANGE.
const byte BTTXPIN =      6;  // Bluetooth serial transmit pin.  DON'T CHANGE.
const byte VREGEN =       7;  // VREGEN pin on bluetooth module
const byte LVERSERVO =    9;  // Left vertical servo pin
const byte RVERSERVO =   10;  // Right vertical servo pin
const byte TOPEYESERVO = 11;  // Top eyelid servo pin
const byte BOTEYESERVO = 12;  // Bottom eyelid servo pin
const byte MP3BUSY =     13;  // MP3 player is busy pin.  Will light Arduino LED when MP3 LED is off (and vice versa)
const byte PRPIN =       14;  // Photoresistor pin (A0)

// Setup all other constants (CHANGE THESE TO MATCH YOUR SETUP)
const byte LASTDIR =                65;  // Last directory # on SD card (10-99)
const byte LEDCOLOR[3] = {255, 100, 5};  // Normal white intensity levels for each ring (inner, middle, outer).  Used on R, G, and B values for white.
const int  MP3DELAY =            10000;  // Delay between playing sound files (ms)
const byte NUMLEDS[3] =     {1, 8, 12};  // Number of LEDs in each ring (inner, middle, outer)
const int  PRLEVEL =               250;  // Photoresistor threshold.  Average reading between light and dark.

// Number of voice files in each directory (CHANGE THESE TO MATCH YOUR SETUP)
//   Can't get DFPLayer to read # of files correctly.  Returns -1
const byte NUMFILES[LASTDIR + 1] = { 0, 38,  8, 12,  3,  0,  0,  0,  0,  0,
                                    14, 16, 26, 13, 11, 24, 13, 14,  5,  7,
                                     7, 12, 12, 14, 10,  5, 34, 19, 16, 22,
                                    15, 12,  5, 10, 18, 23, 18, 16, 17, 25,
                                    23, 26, 17, 20, 14, 15, 11, 13, 15, 12,
                                    13, 11, 16, 16, 14, 12, 12, 16, 13, 10,
                                     4, 11, 21, 15,  4,  8};

// Setup servos (pin, min pos, home pos, max pos, speed) (CHANGE THESE TO MATCH YOUR SETUP)
wServo myServo[5] = {wServo(   HORSERVO, 1200, 1460, 1720,  4),
                     wServo(  LVERSERVO, 1350, 1550, 1750 , 4),
                     wServo(  RVERSERVO, 1280, 1480, 1680,  4),
                     wServo(TOPEYESERVO, 1100, 1350, 1450, 60),
                     wServo(BOTEYESERVO, 1200, 1425, 1550, 60)};

// Setup control variables (Don't change these)
byte currentMode = 0;    // Current mode of operation
byte currentDir;         // Current directory playing (1-99)
byte currentFile;        // Current voice file playing (001-255)
bool isRed;              // The inner ring color is red, not white
byte ledDimmer;          // Control dimming of LED during deactivation
unsigned long modeTimer; // Time to perform next mode operation (ms)
unsigned long mp3End;    // Time mp3 player stopped playing last file (ms)
bool mp3Playing;         // MP3 is playing
unsigned long mp3Start;  // Time mp3 player started playing last file (ms)

// Setup LEDs
Adafruit_NeoPixel leds(NUMLEDS[0] + NUMLEDS[1] + NUMLEDS[2], LEDPIN, NEO_GRB + NEO_KHZ800);

// Setup mp3 player
DFRobotDFPlayerMini mp3;

// Setup extra serial port on Arduino Nano IOT
Uart serialBT(&sercom0, BTRXPIN, BTTXPIN, SERCOM_RX_PAD_1, UART_TX_PAD_0);

void SERCOM0_Handler() {  // Attach the interrupt handler to the SERCOM
  serialBT.IrqHandler();
}

  //****************************************************************************************************************************
 //  SETUP: Initial setup (runs once during start)
//******************************************************************************************************************************
void setup() {

  // Setup pin modes
  pinMode(LEDPIN, OUTPUT);
  pinMode(VREGEN, OUTPUT);
  pinMode(MP3BUSY, INPUT_PULLUP);

  // Reassign pins to SERCOM (extra serial port)
  pinPeripheral(BTRXPIN, PIO_SERCOM_ALT);  // RX for bluetooth
  pinPeripheral(BTTXPIN, PIO_SERCOM_ALT);  // TX for bluetooth

   // Turn off all LEDs
  leds.begin();
  leds.show();

  // Startup all serial ports
  Serial.begin(9600);   // USB port
  Serial1.begin(9600);  // MP3 player (pins 0/1)
  serialBT.begin(9600); // Bluetooth

  // Wait for everything to startup (mainly the Echo Dot to turn on its LEDs)
  delay(6000);

  // Check serial port
  Serial.println("Starting up");
  if (Serial) {
    Serial.println("  - Serial port online");
  }

  // Startup mp3 player
  if (!mp3.begin(Serial1)) {
    Serial.println("  - Trouble starting MP3 player");
    while(true);
  }
  Serial.println("  - MP3 player online");

  // Wait for Echo Dot to setup (lights will go off)
  while(analogRead(PRPIN) > PRLEVEL) {
    delay(500);
  }
  Serial.println("  - Echo Dot online");

  // Enable bluetooth module by putting voltage to VREGEN pin at least 5ms after power is applied
  digitalWrite(VREGEN, HIGH);
  delay(250);
  digitalWrite(VREGEN, LOW);
  delay(1000);
  serialBT.print("MUSIC PLAY\r"); // Set bluetooth to play mode (stops when power is lost).  You could also hard wire to PIO_2 on module.

  // Set mp3 volume (needs a delay after mp3.begin or it will be very loud)
  mp3.volume(18);  // 0-30.  Try to get same level as Echo Dot

  // Setup Arduino Iot Cloud (DON'T USE ANY DELAYS AFTER THIS!!!!!!!!!!!!!)
  initProperties();  // Defined in thingProperties.h
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);  // Connect to Arduino IoT Cloud
  Serial.println("");
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();  // Print out info on cloud

}

  //****************************************************************************************************************************
 //  LOOP: Main loop (continuous)
//******************************************************************************************************************************
void loop() {
  
  ArduinoCloud.update(); // Check for any changes from Echo Dot
  updateEcho();          // Check if Echo Dot is talking
  updateMode();          // Update current mode (scheduler)
  updateMP3();           // Check if done talking  
  updateServos();        // Update servos

}

  //****************************************************************************************************************************
 //  ALLATNEWPOS: All servos are at their new position
//******************************************************************************************************************************
bool allAtNewPos() {

  bool flag = true;
  
  for (int i = 0; i <= 4; i++) {
    if (!myServo[i].atNewPos()) {
      flag = false;
    }
  }

  return flag;
  
}

  //****************************************************************************************************************************
 //  ONWHEATLEYMODECHANGE: Run when Echo Dot changes the mode
//******************************************************************************************************************************
void onWheatleyModeChange() {

  if (!wheatleyMode && currentMode != 0) {
    Serial.println("Change mode to deactivate (from cloud)");
    currentMode = 30;
    modeTimer = millis() + 3000;
  }
  if (wheatleyMode && currentMode != 1) {
    Serial.println("Change mode to activate (from cloud)");
    currentMode = 1;
  }
  
}

  //****************************************************************************************************************************
 //  PLAYMP3: Play MP3 file on SD card card
//******************************************************************************************************************************
void playMP3(byte myDir, byte myFile) {

  Serial.print("  - Play MP3 from dir #");
  Serial.print(myDir);
  Serial.print(" / file #");
  Serial.println(myFile);
  
  mp3.playFolder(myDir, myFile);
  mp3Start = millis();
  mp3Playing = true;

}

  //****************************************************************************************************************************
 //  TURNONLEDS: Turn on LED's (ring #, R value, G value, B value)
//******************************************************************************************************************************
void turnOnLEDs(byte ring, byte r, byte g, byte b) {

    if (ring == 0) {
      for (int i = 0; i < NUMLEDS[0]; i++) {
        leds.setPixelColor(i, r, g, b);
      }
      if (g == 0 && b == 0) {
        isRed = true;
      }
      else {
        isRed = false;
      }
    }

    if (ring == 1) {
      for (int i = NUMLEDS[0]; i < NUMLEDS[0] + NUMLEDS[1]; i++) {
        leds.setPixelColor(i, r, g, b);
      }
    }

    if (ring == 2) {
      for (int i = NUMLEDS[0] + NUMLEDS[1]; i < NUMLEDS[0] + NUMLEDS[1] + NUMLEDS[2]; i++) {
        leds.setPixelColor(i, r, g, b);
      }
    }
    
    leds.show();
  
}

  //****************************************************************************************************************************
 //  UPDATEECHO: Check if Echo Dot is talking
//******************************************************************************************************************************
void updateEcho() {

  if (currentMode >= 10 && currentMode < 20 && analogRead(PRPIN) > PRLEVEL) {
    currentMode = 20;
  }
  
}

  //****************************************************************************************************************************
 //  UPDATEMODE: Main scheduler function to run different modes and switch to the next one.  Can't use delays with Arduino cloud!
//******************************************************************************************************************************
void updateMode() {

  switch(currentMode) {

    // Activate start
    case 1:
      Serial.println("Activate:");
      for (int i = 0; i <= 4; i++) { // Attach all servos and move to home position
        myServo[i].attach();
      }
      randomSeed(micros()); // Set the seed for random # generator
      currentDir = random(10, LASTDIR + 1); // Pick random "chapter" directory to start playing
      currentFile = 0;
      currentMode = 2;
      break;

    // Wait until servos are in position  
    case 2:
      if (allAtNewPos()) {
        modeTimer = millis() + 1000;
        currentMode = 3;
      }
      break;

    // Wait for timer and turn on inner LED ring
    case 3:
      if (millis() >= modeTimer) {
        Serial.println("  - Turn on inner LED ring");
        turnOnLEDs(0, LEDCOLOR[0], LEDCOLOR[0], LEDCOLOR[0]);
        modeTimer = millis() + 1000;
        currentMode = 4;
      }
      break;

    // Wait for timer and turn on middle LED ring
    case 4:
      if (millis() >= modeTimer) {
        Serial.println("  - Turn on middle LED ring");
        turnOnLEDs(1, LEDCOLOR[1], LEDCOLOR[1], LEDCOLOR[1]);
        modeTimer = millis() + 1000;
        currentMode = 5;
      }
      break;

    // Wait for timer, turn on outer LED ring, and say hello
    case 5:
      if (millis() >= modeTimer) {
        Serial.println("  - Turn on outer LED ring");
        turnOnLEDs(2, LEDCOLOR[2], LEDCOLOR[2], LEDCOLOR[2]);
        playMP3(1, random(1, NUMFILES[1] + 1)); // Say hello
        modeTimer = millis() + 1000;
        currentMode = 10;
      }
      break;

    // Active loop: Move eye to random position
    case 10:
      if (allAtNewPos()) {
        //Serial.println("  - Move eye to random position");
        myServo[0].moveRandom();
        myServo[1].moveRandom();
        myServo[2].moveRandom();
        //myServo[3].moveRandom();
        //myServo[4].moveRandom();
        currentMode = 11;
      }
      break;

    // Wait until servos are in position and decide if you want to blink
    case 11:
      if (allAtNewPos()) {
        if (random(0, 4) == 0) {  // 25% chance
          currentMode = 12;
        }
        else {
          modeTimer = millis() + random (0, 3000);
          currentMode = 15;
        }
      }
      break;
      
    // Close eyelids
    case 12:
      //Serial.println("  - Blink");
      myServo[3].moveMin();
      myServo[4].moveMin();
      currentMode = 13;
      break;

    // Wait until servos are in position and set timer
    case 13:
      if (allAtNewPos()) {
        modeTimer = millis() + 100;
        currentMode = 14;
      }
      break;

    // Wait for timer and open eyelids
    case 14:
      if (millis() >= modeTimer) {
        myServo[3].moveHome();
        myServo[4].moveHome();
        currentMode = 15;
      }
      break;
      
    // Wait until servos are in position and play MP3 (if it is time)
    case 15:
      if (allAtNewPos() && millis() >= modeTimer) {
        if (!mp3Playing && millis() - mp3End > MP3DELAY) {
          currentFile++;
          if (currentFile > NUMFILES[currentDir]) {
            Serial.println("  - Done playing all files in dir");
            currentMode = 30;
          }
          else {
            playMP3(currentDir, currentFile);
            currentMode = 10;
          }
        }
        else {
          currentMode = 10;
        }
      }
      break;

    // Warning start
    case 20:
      Serial.println("Warning:");
      mp3.stop();
      myServo[0].moveHome();
      myServo[1].moveHome();
      myServo[2].moveHome();
      myServo[3].moveMax();
      myServo[4].moveMax();
      turnOnLEDs(0, LEDCOLOR[0], 0, 0);  // Turn all LED's bright red
      turnOnLEDs(1, LEDCOLOR[0], 0, 0);
      turnOnLEDs(2, LEDCOLOR[0], 0, 0);
      currentMode = 21;
      break;

    // Wait until servos are in position and Echo Dot to stop talking, then close eyelids
    case 21:
      if (allAtNewPos() && analogRead(PRPIN) < PRLEVEL && millis() >= modeTimer) {
        Serial.println("  - Echo Dot is done talking");
        myServo[3].moveMin();
        myServo[4].moveMin();
        currentMode = 22;
      }
      break;
       
    // Wait until servos are in position and change color back to normal
    case 22:
      if (allAtNewPos()) {
        turnOnLEDs(0, LEDCOLOR[0], LEDCOLOR[0], LEDCOLOR[0]);
        turnOnLEDs(1, LEDCOLOR[1], LEDCOLOR[1], LEDCOLOR[1]);
        turnOnLEDs(2, LEDCOLOR[2], LEDCOLOR[2], LEDCOLOR[2]);
        modeTimer = millis() + 1000;
        currentMode = 23;
      }
      break; 

    // Wait for timer and open eyelids
    case 23:
      if (millis() >= modeTimer) {
        myServo[3].moveHome();
        myServo[4].moveHome();
        currentMode = 24;
      }
      break; 

    // Wait until servos are in position and play a file
    case 24:
      if (allAtNewPos()) {
        playMP3(4, random(1, NUMFILES[4] + 1));
        Serial.println("Activate:");
        currentMode = 10;
      }
      break; 

    // Deactivate start
    case 30:
      Serial.println("Deactivate:");
      mp3.stop();
      if (wheatleyMode) { // Say goodbye
        playMP3(2, random(1, NUMFILES[2] + 1));  // Shut down on own, be nice
      }
      else {
        playMP3(3, random(1, NUMFILES[3] + 1));  // Cloud shut down, be snarky
      }
      currentMode = 31;
      break;

    // Wait for mp3 to finish playing and turn off outer LED ring
    case 31:
      if (!mp3Playing) {
        for (int i = 0; i <= 4; i++) { // Detach all servos
          myServo[i].detach();
        }
        Serial.println("  - Turn off outer LED ring");
        turnOnLEDs(2, 0, 0, 0);
        modeTimer = millis() + 1000;
        currentMode = 32;
      }
      break;

    // Wait for timer and turn off middle LED ring
    case 32:
      if (millis() >= modeTimer) {
        Serial.println("  - Turn off middle LED ring");
        turnOnLEDs(1, 0, 0, 0);
        modeTimer = millis() + 1000;
        ledDimmer = LEDCOLOR[0];
        currentMode = 33;
        Serial.println("  - Turn off inner LED ring");
      }
      break;

    // Wait for timer and turn off inner LED ring slowly
    case 33:
      if (millis() >= modeTimer && ledDimmer > 0) {
        ledDimmer -= 1;
        if (isRed) {  // If in warning mode (red eye)
          turnOnLEDs(0, ledDimmer, 0, 0);
        }
        else {
          turnOnLEDs(0, ledDimmer, ledDimmer, ledDimmer);
        }
        modeTimer = millis() + 25;
      }
      if (ledDimmer <= 0) {
        currentMode = 0;
        wheatleyMode = 0;
      }
      break;

  } //switch(currentMode)
  
}

  //****************************************************************************************************************************
 //  UPDATEMP3: Update status of MP3
//******************************************************************************************************************************
void updateMP3() {

  if (mp3Playing && (millis() - mp3Start > 1000) && (digitalRead(MP3BUSY) == HIGH)) {
    mp3End = millis();
    mp3Playing = false;
  }
  
}

  //****************************************************************************************************************************
 //  UPDATESERVOS: Update servo positions
//******************************************************************************************************************************
void updateServos() {

  if (currentMode > 0 && currentMode < 30) {
    for (int i = 0; i <= 4; i++) {
      myServo[i].update();
    }
  }
  
}
