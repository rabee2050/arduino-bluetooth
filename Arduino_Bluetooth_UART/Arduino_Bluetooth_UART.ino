/*
  Done by TATCO Inc.
  Contacts:
  info@tatco.cc

  Note:
  Make sure the baud rate of the bluetooth module is 9600bps.

  Connection:
  arduino_rx_pin  ------->   Bluetooth_tx_pin
  arduino_tx_pin  ------->   Bluetooth_rx_pin

  Pins that can be used as rx on Mega and Mega 2560:
  10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

  Pins that can be used as rx on on Leonardo:
  8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

  If you are using Bluefruit module then make sure to connect CTS pin to ground.

*/
#define arduino_rx_pin 10  
#define arduino_tx_pin 11

#include <SoftwareSerial.h>

SoftwareSerial mySerial(arduino_rx_pin, arduino_tx_pin); // RX, TX

char mode_action[54];
int mode_val[54];
String mode_feedback;

void setup(void)
{
  Serial.begin(9600);
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB port only
//  }
  Serial.println("Goodnight moon!");
  
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);//you have to change this if you change bluetooth baudrate.
  //mySerial.println("Hello, world?");

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (i == 0 || i == 1 || i == arduino_rx_pin || i == arduino_tx_pin ) {
      mode_action[i] = 'x';
      mode_val[i] = 'x';
    }
    
    else {
      mode_action[i] = 'o';
      mode_val[i] = 0;
      pinMode(i, OUTPUT);
    }
  }
  }
#endif

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    if (i == 0 || i == 1 || i == arduino_rx_pin || i == arduino_tx_pin ) {
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

  if (mySerial.available()) {
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
      if(i!=53)data_status += ",";
  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)//Leo
  for (byte i = 0; i <= 13; i++) {
    data_status += "\"";
      data_status += mode_action[i];
      data_status += "\"";
      if(i!=13)data_status += ",";
  }
#endif
  data_status += "],";

  data_status += "\"mode_val\":[";
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)//Mega
  for (byte i = 0; i <= 53; i++) {
    data_status += mode_val[i];
    if(i!=53)data_status += ",";
  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)//Leo
  for (byte i = 0; i <= 13; i++) {
    data_status += mode_val[i];
    if(i!=13)data_status += ",";
  }
#endif
  data_status += "],";

  data_status += "\"analog\":[";
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)//Mega
  for (byte i = 0; i <= 15; i++) {
        data_status += analogRead(i);
    if(i!=15)data_status += ",";
  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)//Leo
  for (byte i = 0; i <= 5; i++) {
    data_status += analogRead(i);
    if(i!=5)data_status += ",";
  }
#endif
  data_status += "],";
  data_status += "\"mode_feedback\":\"";
  data_status += mode_feedback;
  data_status += "\",";
  data_status += "\"boardname\":\"";
  #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)//Mega
  data_status +="kit_mega";
#endif
#if defined(__AVR_ATmega32U4__)//Leo
  data_status +="kit_leo";
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega16U4__)//UNO
  data_status +="kit_uno";
#endif
  data_status += "\",\"boardstatus\":1";
  data_status += "}";
  mySerial.println(data_status);
  Serial.println(data_status);
  mode_feedback = "";
}
