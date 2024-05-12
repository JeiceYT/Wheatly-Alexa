/*
 * Wheatley Personal Assistant: From the video game Portal 2.  Featuring the Amazon Echo Dot v3.
 *   Use this to find min/home/max servo positions and photoresistor threshold level.
 *   by Steve Turner (arduinokiln@gmail.com)
 */

// Include all libraries
#include <Adafruit_NeoPixel.h>    // LED light control
#include <Servo.h>               // Class for Wheatley servos

// Setup hardware pins (CHANGE THESE TO MATCH YOUR SETUP)
const byte HORSERVO =     2;  // Horizontal servo pin
const byte LEDPIN =       4;  // LED data pin
const byte LVERSERVO =    9;  // Left vertical servo pin
const byte RVERSERVO =   10;  // Right vertical servo pin
const byte TOPEYESERVO = 11;  // Top eyelid servo pin
const byte BOTEYESERVO = 12;  // Bottom eyelid servo pin

// Setup all other constants (CHANGE THESE TO MATCH YOUR SETUP)
const byte NUMLEDS0 =     1;  // Number of LEDs in inner ring
const byte NUMLEDS1 =     8;  // Number of LEDs in middle ring
const byte NUMLEDS2 =    12;  // Number of LEDs in outer ring
                              
// Setup servos: 0 = hor, 1 = left vert, 2 = right ver, 3 = top eye, 4 = bot eye
Servo myServo[5];

// Setup LEDs
Adafruit_NeoPixel leds(NUMLEDS0 + NUMLEDS1 + NUMLEDS2, LEDPIN, NEO_GRB + NEO_KHZ800);

byte white0 = 255;
byte white1 = 100;
byte white2 = 20;

  //****************************************************************************************************************************
 //  SETUP: Initial setup (runs once during start)
//******************************************************************************************************************************
void setup() {

  Serial.begin(9600);

  // Setup pin modes
  pinMode(LEDPIN, OUTPUT);

   // Turn off all LEDs
  leds.begin();

  for (int i = 0; i < NUMLEDS0; i++) {
    leds.setPixelColor(i, white0, white0, white0);
  }
  for (int i = NUMLEDS0; i < NUMLEDS0 + NUMLEDS1; i++) {
    leds.setPixelColor(i, white1, white1, white1);
  }
  for (int i = NUMLEDS0 + NUMLEDS1; i < NUMLEDS0 + NUMLEDS1 + NUMLEDS2; i++) {
    leds.setPixelColor(i, white2, white2, white2);
  }
  
  leds.show();

  // Set each servo to specific points to find min/home/max
  myServo[0].attach(HORSERVO);
  myServo[1].attach(LVERSERVO);
  myServo[2].attach(RVERSERVO);
  myServo[3].attach(TOPEYESERVO);
  myServo[4].attach(BOTEYESERVO);

  myServo[0].writeMicroseconds(1460);  // Change these to find your min/home/max positions
  myServo[1].writeMicroseconds(1480);
  myServo[2].writeMicroseconds(1550);  
  myServo[3].writeMicroseconds(1350);
  myServo[4].writeMicroseconds(1425);  

}

  //****************************************************************************************************************************
 //  LOOP: Main loop (continuous)
//******************************************************************************************************************************
void loop() {

  Serial.println(analogRead(14));  // Watch serial monitor to find your photo resistor threshold
  delay(100);
  
}
