/*
 * Bluetooth Setup Program for Wheatley Personal Assistant: Featuring the Amazon Echo Dot v3
 *   by Steve Turner (arduinokiln@gmail.com)
 */

// Include all libraries
#include <wiring_private.h>  // Needed for SERCOM (extra UART serial port)

// Setup defines for hardware pins (CHANGE THESE TO MATCH YOUR SETUP)
#define BTRXPIN  5  // Bluetooth serial receive pin.  DON'T CHANGE.
#define BTTXPIN  6  // Bluetooth serial transmit pin.  DON'T CHANGE.
#define VREGEN   7  // VREGEN pin on bluetooth module

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
  pinMode(VREGEN, OUTPUT);

  // Reassign pins to SERCOM (extra serial port)
  pinPeripheral(BTRXPIN, PIO_SERCOM_ALT);  // RX for bluetooth
  pinPeripheral(BTTXPIN, PIO_SERCOM_ALT);  // TX for bluetooth

  // Startup all serial ports
  Serial.begin(9600);   // USB port
  serialBT.begin(9600); // Bluetooth

  // Enable bluetooth module by putting voltage to VREGEN pin at least 5ms after power is applied
  delay(5);
  digitalWrite(VREGEN, HIGH);
  delay(250);
  digitalWrite(VREGEN, LOW);
  delay(5000);

  // Return to factory default
  btCommand("RESTORE");
  btCommand("WRITE");
  btCommand("RESET");
  delay(5000);

  // Change settings
  btCommand("SET AUTOCONN=1");
  btCommand("SET CLASSIC_ROLE=1");
  btCommand("SET ENABLE_HFP=OFF");
  btCommand("SET NAME=WHEATLEY");

  // Save
  btCommand("WRITE");

  Serial.println("Done");
 
}

  //****************************************************************************************************************************
 //  LOOP: Main loop (continuous)
//******************************************************************************************************************************
void loop() {


}

  //****************************************************************************************************************************
 //  BTCOMMAND: Send command to bluetooth module
//******************************************************************************************************************************
void btCommand(String myString) {

  Serial.println(myString + " ...");
  serialBT.print(myString + "\r");
  delay(500);
  while(serialBT.available()) {
    Serial.print(char(serialBT.read()));
    delay(25);
  }
  Serial.println("");

}
