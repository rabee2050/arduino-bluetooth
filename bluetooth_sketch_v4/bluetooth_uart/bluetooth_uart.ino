/*
  Title  : Bluetooth Kit
  version: V4.
  Sketch Type: UART.
  Contact: info@tatco.cc
  Done By: TATCO Inc.
  github : https://github.com/rabee2050/arduino-bluetooth
  Youtube: http://tatco.cc

  App - Pro Version:
  iOS    : https://itunes.apple.com/us/app/arduino-bluetooth-kit/id1172121838?mt=8
  Android: https://play.google.com/store/apps/details?id=com.tatco.bluetooth

  App - Free Version:
  iOS    : Coming Soon
  Android: https://play.google.com/store/apps/details?id=com.tatco.bluetoothfree
  

  Release Notes:
  - V1 Created 01 Dec 2016
  - V2 Updated 01 Jun 2017
  - V3 Updated 01 Dec 2017
  - V4 Updated 12 Dec 2018

  Compatibale Bluetooth Module:
  - Adafruit Bluefruit UART , HM-10, CC2541, TI, Redbear, 
  Seeeduino, or any hardware with a "Serial Like" service.


  Note:
  The baud rate of the bluetooth module should be 9600.

  Connection:
  arduino_rx_pin  ------->   Bluetooth_tx_pin
  arduino_tx_pin  ------->   Bluetooth_rx_pin

  Pins that can be used as rx on Mega and Mega 2560:
  10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

  Pins that can be used as rx on on Leonardo:
  8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

  If you are using Bluefruit module then make sure to connect CTS pin to ground.


*/


#include <SoftwareSerial.h>
#include <Servo.h>

#define arduino_rx_pin 10  //must be inturrpt pin
#define arduino_tx_pin 11  //
#define lcdSize 3 //this will define number of LCD on the phone app
int refreshTime = 3; //the app data will be updated every 3 seconds.

SoftwareSerial mySerial(arduino_rx_pin, arduino_tx_pin); // RX, TX

char pinsMode[54];
int pinsValue[54];
String feedBack;
String lcd[lcdSize];
unsigned long last = millis();
Servo servoArray[54];
String boardType;
String protectionPassword = "";
String appBuildVersion = "4.0";

void setup(void)
{
  Serial.begin(9600);// set the data rate for the Hardware Serial port
  mySerial.begin(9600);//you have to change this if you change bluetooth baudrate.
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
  String data = mySerial.readStringUntil('/');
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
    if (i == 0 || i == 1 || i == arduino_rx_pin || i == arduino_tx_pin ) {
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
    if (i == 0 || i == 1 || i == arduino_rx_pin || i == arduino_tx_pin ) {
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
