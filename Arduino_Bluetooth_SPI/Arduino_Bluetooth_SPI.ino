/*
  Done by TATCO Inc.
  Contacts:
  info@tatco.cc

  Note:
  This sketch compatible with Bluefruit SPI version.

  Connection:
  BLUEFRUIT_MOSI                 Arduino MOSI
  BLUEFRUIT_MISO                 Arduino MISO
  BLUEFRUIT_CLK                  Arduino CLK
  BLUEFRUIT_SPI_CS               8
  BLUEFRUIT_SPI_IRQ              7
  BLUEFRUIT_SPI_RST              4

  If you are using Feather32u4 then Pin A0 used to measure 
  battery status, take a wire from Battery pin through 
  voltage devider then connect to pin A0.(You have to uncomment 
  few lines at void allstatus).


*/

#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
#include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"


char mode_action[54];
int mode_val[54];
String mode_feedback;

#define BUFSIZE                        128   // Size of the read buffer for incoming data
#define VERBOSE_MODE                   true  // If set to 'true' enables debug output

#define BLUEFRUIT_SPI_CS               8
#define BLUEFRUIT_SPI_IRQ              7
#define BLUEFRUIT_SPI_RST              4    // Optional but recommended, set to -1 if unused


#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"

Adafruit_BluefruitLE_SPI mySerial(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}


void setup(void)
{
  //  while (!Serial);  // required for Flora & Micro
  //  delay(500);

  Serial.begin(9600);

  if ( !mySerial.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    Serial.println(F("Performing a factory reset: "));
    if ( ! mySerial.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }
  mySerial.echo(false);
  mySerial.info();
  mySerial.verbose(false);  // debug info is a little annoying after this point!
  while (! mySerial.isConnected()) {
    delay(500);
  }

  mySerial.setMode(BLUEFRUIT_MODE_DATA);

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (i == 0 || i == 1 || i == BLUEFRUIT_SPI_RST  || i == BLUEFRUIT_SPI_IRQ || i == BLUEFRUIT_SPI_CS ) {
      mode_action[i] = 'x';
      mode_val[i] = 'x';
    }
    else {
      mode_action[i] = 'o';
      mode_val[i] = 0;
      pinMode(i, OUTPUT);
    }
  }

#endif

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    if (i == 0 || i == 1 || i == BLUEFRUIT_SPI_RST  || i == BLUEFRUIT_SPI_IRQ || i == BLUEFRUIT_SPI_CS ) {
      mode_action[i] = 'x';
      mode_val[i] = 'x';
    }
    else {
      mode_action[i] = 'o';
      mode_val[i] = 0;
      pinMode(i, OUTPUT);
    }
  }
#endif
}

void update_input() {
  for (int i = 0; i < sizeof(mode_action); i++) {
    if (mode_action[i] == 'i') {
      mode_val[i] = digitalRead(i);

    }
  }
}

void loop(void)
{
  if ( mySerial.available() )
  {
    update_input();
    process();
  }
}

void process() {

  String command = mySerial.readStringUntil('/');

  if (command == "digital") {
    digitalCommand();
  }

  if (command == "analog") {
    analogCommand();
  }

  if (command == "mode") {
    modeCommand();
  }

  if (command == "allonoff") {
    allonoff();
  }

  if (command == "allstatus") {
    allstatus();
  }


}


void digitalCommand() {
  int pin, value;
  pin = mySerial.parseInt();
  if (mySerial.read() == '/') {
    value = mySerial.parseInt();
    digitalWrite(pin, value);
    mode_val[pin] = value;
  }
}

void analogCommand() {
  int pin, value;
  pin = mySerial.parseInt();
  if (mySerial.read() == '/') {
    value = mySerial.parseInt();
    analogWrite(pin, value);
    mode_val[pin] = value;
  }

}


void modeCommand() {
  int pin;
  pin = mySerial.parseInt();
  mode_feedback = "";
  String mode = mySerial.readStringUntil(' ');
  if (mode == "/input") {
    pinMode(pin, INPUT);
    mode_action[pin] = 'i';
    mode_feedback += "D";
    mode_feedback += pin;
    mode_feedback += " set as INPUT!";
  }

  if (mode == "/output") {
    pinMode(pin, OUTPUT);
    mode_action[pin] = 'o';
    mode_feedback += "D";
    mode_feedback += pin;
    mode_feedback += " set as OUTPUT!";
  }

  if (mode == "/pwm") {
    pinMode(pin, OUTPUT);
    mode_action[pin] = 'p';
    mode_feedback += "D";
    mode_feedback += pin;
    mode_feedback += " set as PWM!";
  }

  allstatus();

}

void allonoff() {
  int pin, value;
  value = mySerial.parseInt();
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    if (mode_action[i] == 'o') {
      digitalWrite(i, value);
      mode_val[i] = value;
    }
  }
#endif
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (mode_action[i] == 'o') {
      digitalWrite(i, value);
      mode_val[i] = value;
    }
  }
#endif
  allstatus();

}


void allstatus() {

  String data_status;
  data_status += "{";

  data_status += "\"mode\":[";
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)//Mega
  for (byte i = 0; i <= 53; i++) {
    data_status += "\"";
    data_status += mode_action[i];
    data_status += "\"";
    if (i != 53)data_status += ",";
  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)//Leo
  for (byte i = 0; i <= 13; i++) {
    data_status += "\"";
    data_status += mode_action[i];
    data_status += "\"";
    if (i != 13)data_status += ",";
  }
#endif
  data_status += "],";

  data_status += "\"mode_val\":[";
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)//Mega
  for (byte i = 0; i <= 53; i++) {
    data_status += mode_val[i];
    if (i != 53)data_status += ",";
  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)//Leo
  for (byte i = 0; i <= 13; i++) {
    data_status += mode_val[i];
    if (i != 13)data_status += ",";
  }
#endif
  data_status += "],";

  data_status += "\"analog\":[";
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)//Mega
  for (byte i = 0; i <= 15; i++) {
    data_status += analogRead(i);
    if (i != 15)data_status += ",";
  }
#endif
  //uno+pro //nano //leo+lilypad //Feather 
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)//Leo
  for (byte i = 0; i <= 5; i++) {
    //    if (i == 0) {   //Uncomment this if you want to measure battery status via pin A0
    //      float measuredvbat = analogRead(i);
    //      measuredvbat *= 2;
    //      measuredvbat *= 3.3;
    //      measuredvbat /= 1024;
    //      measuredvbat -= 3.2;
    //      measuredvbat *= 100;
    //      data_status += abs(measuredvbat);
    //    } else {
    data_status += analogRead(i);
    //}
    if (i != 5)data_status += ",";
  }
#endif
  data_status += "],";
  data_status += "\"mode_feedback\":\"";
  data_status += mode_feedback;
  data_status += "\",";
  data_status += "\"boardname\":\"";
  data_status += "kit_feather";
  data_status += "\",\"boardstatus\":1";
  data_status += "}";
  mySerial.println(data_status);
  Serial.print(data_status);
  mode_feedback = "";
}
