/*
  in use
  L298N Dual H-Bridge Motor Controller to open/close the door
  relay (to save battery power) connecter to pin 1
  Pro Mini 328 3.3v 8MHz

  Silicon Labs CP210x USB to UART Bridge pinout <-|-> Pro Mini RobotDyn 328 3.3v 8MHz
  DRT - DTR
  RXD - TXO
  TXD - RXI
  +3.3V - VCC
  GND - GRN
*/

#include <JeeLib.h>
ISR(WDT_vect) {
  Sleepy::watchdogEvent();
}


int motor1a_pin = 3;
int motor1b_pin = 4;
int motor1speed_pin = 7;  // motor1speed_pin needs to be a PWM
int senseClosed_pin = 5;
int senseOpened_pin = 6;
int light_pin = 14; //A0
int doorState = 0; // -1 is closed, 0 is unknown, 1 is opened
int relay = 13;
int maxDoorMove = 30000;

void setup() {
  pinMode(motor1a_pin, OUTPUT);
  pinMode(motor1b_pin, OUTPUT);
  pinMode(motor1speed_pin, OUTPUT);
  pinMode(senseClosed_pin, INPUT);
  pinMode(senseOpened_pin, INPUT);
  pinMode(relay, OUTPUT);
}

void loop() {
  bool isDark = IsDark();
  if (isDark && doorState != -1) {
    door(false); // close
  }
  else if (!isDark && doorState != 1) {
    door(true); // open
  }
  Sleepy::loseSomeTime(900000); // next check is 15 minutes later
}


bool IsDark()
{
 int light = 0;
 int test1 = analogRead(light_pin);
  delay(100);
 int test2 = analogRead(light_pin);

  if (test1 < 25 && test2 < 25) {
    light = 0;
  }
  else if (test1 > 145 && test2 > 145) {
    light = 200;
  }
  else {
    light = (test1 + test2) / 2;
  }

  // light < 25: night
  // light > 145: day
  // int light =  analogRead(light_pin);
  int threshold = 85 + -60 * doorState;
  if (light < threshold) {
    return true;
  }
  else {
    return false;
  }
}


void door(bool open)
{
  // give power to motor driver
  digitalWrite(relay, 1);

  //reset_millis
  extern volatile unsigned long timer0_millis, timer0_overflow_count;
  noInterrupts();
  timer0_millis = timer0_overflow_count = 0;
  interrupts();

  int sencePin = open ? senseOpened_pin : senseClosed_pin;
  analogWrite(motor1speed_pin, 1200);
  digitalWrite(motor1a_pin, !open);
  digitalWrite(motor1b_pin, open);

  while (digitalRead(sencePin) == LOW && millis() < maxDoorMove) {
    delay(20);
  }

  // let run door closing another 2 sec after door sensor alreary reported "door already closed"
  if (!open) {
    delay(2000);
  }

  //DoorStop
  analogWrite(motor1speed_pin, 0);
  digitalWrite(motor1a_pin, LOW);
  digitalWrite(motor1b_pin, LOW);
  digitalWrite(relay, 0);
  doorState = open ? 1 : -1;
}
