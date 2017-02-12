/*
  Done by TATCO Inc.
  Contacts:
  info@tatco.cc

  Note:
  Make sure the baud rate of the bluetooth is 9600, and it comes
  by default if it is new kit.

  Connection:
  arduino_rx_pin  ------->   Bluetooth_tx_pin
  arduino_tx_pin  ------->   Bluetooth_rx_pin


*/


#include <SoftwareSerial.h>
#include <Servo.h>

#define arduino_rx_pin 10  //must be inturrpt pin
#define arduino_tx_pin 11
#define lcd_size 3 //this will define number of LCD on the phone app
#define refresh_time  5 //the data will be updated on the app every 5 seconds.

SoftwareSerial mySerial(arduino_rx_pin, arduino_tx_pin); // RX, TX

char mode_action[54];
int mode_val[54];
String mode_feedback;
String lcd[lcd_size];
unsigned long last = millis();
Servo myServo[54];

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



void loop(void)
{
  lcd[0] = "Test 1 LCD";// you can send any data to your mobile phone.
  lcd[1] = "Test 2 LCD";// you can send any data to your mobile phone.
  lcd[2] = analogRead(1);//  send analog value of A1
 
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

  if (command == "analog") {
    analogCommand();
  }

  if (command == "mode") {
    modeCommand();
  }

  if (command == "servo") {
    servo();
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

void servo() {
  int pin, value;
  pin = mySerial.parseInt();
  if (mySerial.read() == '/') {
    value = mySerial.parseInt();
    myServo[pin].write(value);
    mode_val[pin] = value;
  }
}

void modeCommand() {
  int pin;
  pin = mySerial.parseInt();
  mode_feedback = "";
  if (mySerial.read() == '/') {

    String mode = mySerial.readStringUntil('\r');
    if (mode == "input") {
      pinMode(pin, INPUT);
      mode_action[pin] = 'i';
      mode_feedback += "D";
      mode_feedback += pin;
      mode_feedback += " set as INPUT!";
    }

    if (mode == "output") {
      pinMode(pin, OUTPUT);
      mode_action[pin] = 'o';
      mode_feedback += "D";
      mode_feedback += pin;
      mode_feedback += " set as OUTPUT!";
    }

    if (mode == "pwm") {
      pinMode(pin, OUTPUT);
      mode_action[pin] = 'p';
      mode_feedback += "D";
      mode_feedback += pin;
      mode_feedback += " set as PWM!";
    }

    if (mode == "servo") {
      myServo[pin].attach(pin);
      mode_action[pin] = 's';
      mode_feedback += "D";
      mode_feedback += pin;
      mode_feedback += " set as SERVO!";
    }
    allstatus();
  }
}

void allonoff() {
  int pin, value;
  value = mySerial.parseInt();
  for (byte i = 0; i < sizeof(mode_action); i++) {
    if (mode_action[i] == 'o') {
      digitalWrite(i, value);
      mode_val[i] = value;
    }
  }
}

void update_input() {
  for (int i = 0; i < sizeof(mode_action); i++) {
    if (mode_action[i] == 'i') {
      mode_val[i] = digitalRead(i);

    }
  }
}

void update_app() {
  int refreshVal=refresh_time*1000;
  if (millis() - last > refreshVal) {
    allstatus();
    last = millis();
  }
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
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)//Leo
  for (byte i = 0; i <= 5; i++) {
    data_status += analogRead(i);
    if (i != 5)data_status += ",";
  }
#endif
  data_status += "],";

  data_status += "\"lcd\":[";
  for (byte i = 0; i <= lcd_size-1; i++) {
    data_status += "\"";
    data_status += lcd[i];
    data_status += "\"";
    if (i != lcd_size-1)data_status += ",";
  }
  data_status += "],";

  data_status += "\"mode_feedback\":\"";
  data_status += mode_feedback;
  data_status += "\",";
  
  data_status += "\"boardname\":\"";
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)//Mega
  data_status += "kit_mega";
#endif
#if defined(__AVR_ATmega32U4__)//Leo
  data_status += "kit_leo";
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega16U4__)//UNO
  data_status += "kit_uno";
#endif
  data_status += "\",\"boardstatus\":1";
  data_status += "}";
  mySerial.println(data_status);
  Serial.println(data_status);
  mode_feedback = "";
}
