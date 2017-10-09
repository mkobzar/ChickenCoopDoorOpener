// For L298N Dual H-Bridge Motor Controller
// pro mini is board in use

#include <JeeLib.h> // Low power functions library
ISR(WDT_vect) {
  Sleepy::watchdogEvent();  // Setup the watchdog
}
//#define DEBUG

// Motor 1 // D2, D3, D9 => works with nodemcu
int in1_pin = PCINT19;
int in2_pin = PCINT20;
int enA_pin = PD7; // enA_pin needs to be a PWM
int senseClosed_pin = PD5;
int senseOpened_pin = PD6;
int light_pin = A0;
int doorState = 0; // -1 is closed, 0 is unknown, 1 is opened

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
#endif
  pinMode(in1_pin, OUTPUT);
  pinMode(in2_pin, OUTPUT);
  pinMode(enA_pin, OUTPUT);
  pinMode(senseClosed_pin,  INPUT);
  pinMode(senseOpened_pin,  INPUT);
}


void loop() {
  bool isDark = IsDark();
#ifdef DEBUG
  Serial.println("loop begin");
  Serial.print("doorState=");
  Serial.print(doorState);
  Serial.print(", isDark=");
  Serial.println(isDark);
#endif
  if (isDark && doorState != -1) {
    DoorClose();
#ifdef DEBUG
    Serial.println("closed");
#endif
  }
  else if (!isDark && doorState != 1) {
    DoorOpen();
#ifdef DEBUG
    Serial.println("opened");
#endif
  } else {
#ifdef DEBUG
    Serial.println("deep sleep");
#endif
    Sleepy::loseSomeTime(120000);
  }
}


bool IsDark()
{
  int light = analogRead(light_pin);
  int threshold = 200 + -100 * doorState;
#ifdef DEBUG
  Serial.print(light);
#endif
  if (light < threshold ) {
#ifdef DEBUG
    Serial.println(" = night");
#endif
    return true;
  }
  else {
#ifdef DEBUG
    Serial.println(" = day");
#endif
    return false;
  }
}

void DoorOpen()
{
#ifdef DEBUG
  Serial.println("DoorOpen()");
#endif
  analogWrite(enA_pin, 1200);
  digitalWrite(in1_pin, LOW);
  digitalWrite(in2_pin, HIGH);
  while (digitalRead(senseOpened_pin) != HIGH) {
    delay(20);
  }
  DoorStop();
  doorState = 1;
}

void DoorClose()
{
#ifdef DEBUG
  Serial.println("DoorClose()");
#endif
  analogWrite(enA_pin, 1200);
  digitalWrite(in2_pin, LOW);
  digitalWrite(in1_pin, HIGH);
  while (digitalRead(senseClosed_pin) != HIGH) {
    delay(20);
  }
  DoorStop();
  doorState = -1;
}

void DoorStop()
{
#ifdef DEBUG
  Serial.println("DoorStop()");
#endif
  analogWrite(enA_pin, 0);
  digitalWrite(in1_pin, LOW);
  digitalWrite(in2_pin, LOW);
}

