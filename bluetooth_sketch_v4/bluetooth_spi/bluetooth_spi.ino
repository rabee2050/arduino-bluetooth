/*
  Done by TATCO Inc.
  Contacts:
  info@tatco.cc

  Note:
  This sketch compatible with Bluefruit SPI and Feather Bluefruit versions.

  Connection:
  BLUEFRUIT_MOSI                 Arduino MOSI
  BLUEFRUIT_MISO                 Arduino MISO
  BLUEFRUIT_CLK                  Arduino CLK
  BLUEFRUIT_SPI_CS               8
  BLUEFRUIT_SPI_IRQ              7
  BLUEFRUIT_SPI_RST              4

*/

#include <Arduino.h>
#include <Servo.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
#include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#define lcdSize 3 //this will define number of LCD on the phone app
int refreshTime = 3; //the app data will be updated every 3 seconds.

char pinsMode[54];
int pinsValue[54];
String feedBack;
String lcd[lcdSize];
unsigned long last = millis();
Servo servoArray[54];
String boardType;
String protectionPassword = "";
String appBuildVersion = "4.0";

#define BUFSIZE                        256   // Size of the read buffer for incoming data
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

  boardInit();

}

void loop(void)
{
  lcd[0] = "Test 1 LCD";// you can send any data to your mobile phone.
  lcd[1] = String(analogRead(1));// you can send any data to your mobile phone.
  lcd[2] = String(random(1, 100)); // you can send any data to your mobile phone.
  if ( mySerial.available() )
  {
    process();
  }
  update_input();
  update_app();//will update all values in the mobile app.
}

void process() {

  String command = mySerial.readStringUntil('/');

  if (command == "digital") {
    digitalCommand();
  }

  if (command == "pwm") {
    pwmCommand();
  }

  if (command == "servo") {
    servoCommand();
  }

  if (command == "terminal") {
    terminalCommand();
  }

  if (command == "mode") {
    modeCommand();
  }

  if (command == "allonoff") {
    allonoff();
  }

  if (command == "refresh") {
    refresh();
  }

  if (command == "password") {
    changePassword();
  }

  if (command == "allstatus") {
    feedBack = "refresh/";
    allstatus();
  }
}

void changePassword() {
  String data = mySerial.readStringUntil('\r');
  protectionPassword = data;
}

void terminalCommand() {//Here you recieve data form app terminal

  String data = mySerial.readStringUntil('/');
  Serial.println(data);

  String replyToApp = "Ok from Arduino"; //It can be change to any thing

  feedBack = "terminal/" + replyToApp; //dont change this line.
  allstatus();
}

void digitalCommand() {
  int pin, value;
  pin = mySerial.parseInt();
  if (mySerial.read() == '/') {
    value = mySerial.parseInt();
    digitalWrite(pin, value);
    pinsValue[pin] = value;
  }
}

void pwmCommand() {
  int pin, value;
  pin = mySerial.parseInt();
  if (mySerial.read() == '/') {
    value = mySerial.parseInt();
    analogWrite(pin, value);
    pinsValue[pin] = value;
  }

}

void servoCommand() {
  int pin, value;
  pin = mySerial.parseInt();
  if (mySerial.read() == '/') {
    value = mySerial.parseInt();
    servoArray[pin].write(value);
    pinsValue[pin] = value;
  }
}

void modeCommand() {
  int pin;
  pin = mySerial.parseInt();
  //  mode_feedback = "";
  if (mySerial.read() == '/') {
    String mode = mySerial.readStringUntil('/');
    if (mode == "output") {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, 0);
      pinsMode[pin] = 'o';
      pinsValue[pin] = 0;
    }
    if (mode == "push") {
      pinsMode[pin] = 'm';
      pinsValue[pin] = 0;
      pinMode(pin, OUTPUT);
      digitalWrite(pin, 0);
    }
    if (mode == "schedule") {
      pinsMode[pin] = 'c';
      pinsValue[pin] = 0;
      pinMode(pin, OUTPUT);
      digitalWrite(pin, 0);
    }

    if (mode == "input") {
      pinsMode[pin] = 'i';
      pinsValue[pin] = 0;
      pinMode(pin, INPUT);
    }

    if (mode == "pwm") {
      pinsMode[pin] = 'p';
      pinsValue[pin] = 0;
      pinMode(pin, OUTPUT);
      analogWrite(pin, 0);
    }

    if (mode == "servo") {
      pinsMode[pin] = 's';
      pinsValue[pin] = 0;
      servoArray[pin].attach(pin);
      servoArray[pin].write(0);
    }
    feedBack = "mode/" + mode + "/" + pin + "/" + pinsValue[pin];
    allstatus();
  }
}

void allonoff() {
  int pin, value;
  value = mySerial.parseInt();
  for (byte i = 0; i < sizeof(pinsMode); i++) {
    if (pinsMode[i] == 'o') {
      digitalWrite(i, value);
      pinsValue[i] = value;
    }
  }
}

void refresh() {
  int value;
  value = mySerial.parseInt();
  refreshTime = value;
  allstatus();

}



void update_input() {
  for (int i = 0; i < sizeof(pinsMode); i++) {
    if (pinsMode[i] == 'i') {
      pinsValue[i] = digitalRead(i);
    }
  }
}

void update_app() {

  if (refreshTime != 0) {
    int refreshVal = refreshTime * 1000;
    if (millis() - last > refreshVal) {
      allstatus();
      last = millis();
    }
  }
}


void allstatus() {
  int digitalArraySize, analogArraySize;
  if (boardType == "mega") {
    digitalArraySize = 53;
    analogArraySize = 15;
  } else {
    digitalArraySize = 13;
    analogArraySize = 5;
  }
  String dataResponse;
  dataResponse += "{";

  dataResponse += "\"m\":[";//m for mode
  for (byte i = 0; i <= digitalArraySize; i++) {
    dataResponse += "\"";
    dataResponse += pinsMode[i];
    dataResponse += "\"";
    if (i != digitalArraySize)dataResponse += ",";
  }
  dataResponse += "],";

  dataResponse += "\"v\":[";//v for value
  for (byte i = 0; i <= digitalArraySize; i++) {
    dataResponse += pinsValue[i];
    if (i != digitalArraySize)dataResponse += ",";
  }
  dataResponse += "],";

  dataResponse += "\"a\":[";//a for analog value
  for (byte i = 0; i <= analogArraySize; i++) {
    dataResponse += analogRead(i);
    if (i != analogArraySize)dataResponse += ",";
  }
  dataResponse += "],";

  dataResponse += "\"l\":[";// for lcd
  for (byte i = 0; i <= lcdSize - 1; i++) {
    dataResponse += "\"";
    dataResponse += lcd[i];
    dataResponse += "\"";
    if (i != lcdSize - 1)dataResponse += ",";
  }
  dataResponse += "],";

  dataResponse += "\"t\":\""; //t for Board Type .
  dataResponse += boardType;
  dataResponse += "\",";
  dataResponse += "\"f\":\""; //t for Board Type .
  dataResponse += feedBack;
  dataResponse += "\",";
  dataResponse += "\"r\":\""; //t for Board Type .
  dataResponse += refreshTime;
  dataResponse += "\",";
  dataResponse += "\"b\":\""; //b for app build version .
  dataResponse += appBuildVersion;
  dataResponse += "\",";
  dataResponse += "\"p\":\""; // p for Password.
  dataResponse += protectionPassword;
  dataResponse += "\"";
  dataResponse += "}";
  mySerial.println(dataResponse);
  feedBack = "";
}

void boardInit() {
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (i == 0 || i == 1 || i == BLUEFRUIT_SPI_RST  || i == BLUEFRUIT_SPI_IRQ || i == BLUEFRUIT_SPI_CS ) {
      pinsMode[i] = 'x';
      pinsValue[i] = 'x';
    }
    else {
      pinsMode[i] = 'o';
      pinsValue[i] = 0;
      pinMode(i, OUTPUT);
    }
  }

#endif

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    if (i == 0 || i == 1 || i == BLUEFRUIT_SPI_RST  || i == BLUEFRUIT_SPI_IRQ || i == BLUEFRUIT_SPI_CS ) {
      pinsMode[i] = 'x';
      pinsValue[i] = 'x';
    }
    else {
      pinsMode[i] = 'o';
      pinsValue[i] = 0;
      pinMode(i, OUTPUT);
    }
  }
#endif

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  boardType = "uno";
#elif defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__) || defined(__SAM3X8E__)
  boardType = "mega";
#elif defined(__AVR_ATmega32U4__)
  boardType = "leo";
#else
  boardType = "uno";
#endif
  feedBack = "refresh/";
  allstatus();

}
