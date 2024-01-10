#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <Wire.h>  
#include "HT_SSD1306Wire.h"
#include "ezButton.h"

const int ledPin = 35;    // the number of the LED pin
int ledState = LOW;        // the current state of the output pin
String ledstring = "OFF";
int ledspacing = 0;

// ##### screen display setup ######
SSD1306Wire  factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED); // addr , freq , i2c group , resolution , rst


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

static RadioEvents_t RadioEvents;

int16_t txNumber;

int16_t rssi,rxSize;

bool lora_idle = true;

void setup() {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, ledState);
    
    // set serial connection
    Serial.begin(115200);
    
    // Initiallize screen
    factory_display.init();
    factory_display.clear();
    factory_display.display();

    //factory_display.drawString(0, 0, "Test Start");
    factory_display.display();
    factory_display.clear();
    

    // LoRa setup
    Mcu.begin();
    
    txNumber=0;
    rssi=0;
  
    RadioEvents.RxDone = OnRxDone;
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                               LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                               LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                               0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
}



void loop()
{
  if(lora_idle)
  {
    lora_idle = false;
    Serial.println("into RX mode");
    Radio.Rx(0);
  }
  Radio.IrqProcess( );
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    rssi=rssi;
    rxSize=size;
    memcpy(rxpacket, payload, size );
    rxpacket[size]='\0';
    Radio.Sleep( );
    Serial.printf("\r\nreceived packet \"%s\" with rssi %d , length %d\r\n",rxpacket,rssi,rxSize);
    lora_idle = true;
    
    // Toggle Led 
    ledState = !ledState;
    digitalWrite(ledPin, ledState);

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
    
    

}