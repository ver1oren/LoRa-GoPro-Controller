/*
  Debounce

  Each time the input pin goes from LOW to HIGH (e.g. because of a push-button
  press), the output pin is toggled from LOW to HIGH or HIGH to LOW. There's a
  minimum delay between toggles to debounce the circuit (i.e. to ignore noise).

  The circuit:
  - LED attached from pin 13 to ground through 220 ohm resistor
  - pushbutton attached from pin 2 to +5V
  - 10 kilohm resistor attached from pin 2 to ground

  - Note: On most Arduino boards, there is already an LED on the board connected
    to pin 13, so you don't need any extra components for this example.

  created 21 Nov 2006
  by David A. Mellis
  modified 30 Aug 2011
  by Limor Fried
  modified 28 Dec 2012
  by Mike Walters
  modified 30 Aug 2016
  by Arturo Guadalupi

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Debounce
*/

#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <Wire.h>  
#include "HT_SSD1306Wire.h"
#include "ezButton.h"


// ##### button stuff #####


ezButton button(0);  // create ezButton object that attach to pin 0;

const int ledPin = 35;    // the number of the LED pin

// Variables will change:
int ledState = LOW;        // the current state of the output pin
String ledstring = "OFF";
int ledspacing = 0;





// ##### screen display setup ######
SSD1306Wire  factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED); // addr , freq , i2c group , resolution , rst



// ######  LoRa seetup #######

#define RF_FREQUENCY                                915000000 // Hz

//#define TX_OUTPUT_POWER                             5        // dBm
#define TX_OUTPUT_POWER                             30        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
//#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_SPREADING_FACTOR                       11         // [SF7..SF12]
//#define LORA_CODINGRATE                             1         // [1: 4/5,
#define LORA_CODINGRATE                             4         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 12 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

double txNumber;

bool lora_idle=true;

static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );


void setup() {
  //pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  button.setDebounceTime(50); // set debounce time to 50 milliseconds

  // set initial LED state
  digitalWrite(ledPin, ledState);


  // set serial connection
  Serial.begin(115200);
  Serial.println("Setup");

  // Initiallize screen
  factory_display.init();
  factory_display.clear();
  factory_display.display();

  //factory_display.drawString(0, 0, "Test Start");
	factory_display.display();
  factory_display.clear();


  // setup for LoRa stuff
  Mcu.begin();
	
  txNumber=0;

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  
  Radio.Init( &RadioEvents );
  Radio.SetChannel( RF_FREQUENCY );
  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                  LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                  LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                  true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 


}



void loop() {
  // read the state of the switch into a local variable:
  //int reading = digitalRead(buttonPin);
  button.loop(); // MUST call the loop() function first

  if(button.isPressed()) {
 
    // Set the led state variable which gets updated at the end of the loop
    ledState = !ledState;

    // update the screen
    factory_display.clear();
    ledstring = "Off";
    ledspacing = 0;
    if (ledState == HIGH) {
      ledstring = "ON";
      ledspacing = 30;
    }
    factory_display.drawString(0, 0, "Led:  ");
    factory_display.drawString(ledspacing, 8, ledstring);
    factory_display.display();
    
    Serial.println("Button Pressed");

    // Send the LoRa packet
    txNumber += 0.01;
    sprintf(txpacket,"msg %0.2f",txNumber);  //start a package
  
    Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));

    Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out	
    lora_idle = false;
    Radio.IrqProcess( );

  }

  // set the LED:
  digitalWrite(ledPin, ledState);

}


void OnTxDone( void )
{
	Serial.println("TX done......");
	lora_idle = true;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    Serial.println("TX Timeout......");
    lora_idle = true;
}
