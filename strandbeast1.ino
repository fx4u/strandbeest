#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
  #include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "BluefruitConfig.h"

/* ----- Motorshield ------- */
#include <Adafruit_MotorShield.h>
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_DCMotor *myMotor1 = AFMS.getMotor(3);
Adafruit_DCMotor *myMotor2 = AFMS.getMotor(4);

#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
float parsefloat(uint8_t *buffer);
void printHex(const uint8_t * data, const uint32_t numBytes);
extern uint8_t packetbuffer[];

void setup(void){
  Serial.begin(115200);

  /* ----- Motorshield ------- */
  AFMS.begin();
  
  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if (!ble.begin(VERBOSE_MODE)){
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if (FACTORYRESET_ENABLE){
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if (! ble.factoryReset()){
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in Controller mode"));
  Serial.println(F("Then activate/use the sensors, color picker, game controller, etc!"));
  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
      delay(500);
  }

  Serial.println(F("******************************"));

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  // Set Bluefruit to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("******************************"));
}

void loop(void)
{
  /* Wait for new data to arrive */
  uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
  if (len == 0) return;

  int speed = 150;

  // Buttons
  if (packetbuffer[1] == 'B') {
    uint8_t buttnum = packetbuffer[2] - '0';
    boolean pressed = packetbuffer[3] - '0';

    /* STOP */
    if(buttnum == 1 and pressed == 1){
      myMotor1->setSpeed(0);
      myMotor2->setSpeed(0); 
      Serial.println("STOP");
    } 

    if(buttnum == 2 and pressed == 1){
      myMotor1->setSpeed(100);
      myMotor2->setSpeed(100); 
      Serial.println("SPEED 100");
    } 

    if(buttnum == 3 and pressed == 1){
      myMotor1->setSpeed(180);
      myMotor2->setSpeed(180); 
      Serial.println("SPEED 180");
    }  

     if(buttnum == 4 and pressed == 1){
      myMotor1->setSpeed(250);
      myMotor2->setSpeed(250); 
      Serial.println("SPEED 250");
    }  
    
    /* Vorwaerts */
    if(buttnum == 5 and pressed == 1){
      myMotor1->run(FORWARD);   
      myMotor2->run(FORWARD);
    }
  
    /* Rueckwaerts */
    if(buttnum == 6 and pressed == 1){
      myMotor1->run(BACKWARD);   
      myMotor2->run(BACKWARD);
    }
  
    /* Links */
    if(buttnum == 7 and pressed == 1){
      myMotor1->run(FORWARD);   
      myMotor2->run(BACKWARD);
    }

    /* Rechts */
    if(buttnum == 8 and pressed == 1){
      myMotor1->run(BACKWARD);   
      myMotor2->run(FORWARD);
      Serial.println("RIGHT");
    }
    
    Serial.print ("Button "); Serial.print(buttnum);
    if (pressed) {
      Serial.println(" pressed");
    } else {
      Serial.println(" released");
    }     
  }
}
