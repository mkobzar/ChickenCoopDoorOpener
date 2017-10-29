// for pro mini is board with L298N Dual H-Bridge Motor Controller
/*
Silicon Labs CP210x USB to UART Bridge pinout <-|-> Pro Mini RobotDyn 328 3.3v
DRT - DTR
RXD - TXO
TXD - RXI
+5V - RAW
GND - GRN (next to RAW)

Silicon Labs CP210x USB to UART Bridge pinout <-|-> Pro Mini RobotDyn 328 3.3v (pinouts\promini328p5_top.jpg)
DRT - GRD (above TX)
RXD - TX
TXD - RX
+5V - RAW (or VCC)
GND - GRN (next to RAW or next to VCC)


Silicon Labs CP210x USB to UART Bridge pinout <-|-> Pro Mini RobotDyn 328 5v (checkenCoop) (pinouts\promini2.png)
DRT - DTR
RXD - TXO
TXD - RXI
+5V - VCC
GND - GRN
*/

// #define DEBUG

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

void setup() {
#ifdef DEBUG
	delay(3000);
	Serial.begin(9600);
	Serial.println("Serial.begin");
#endif
	pinMode(motor1a_pin, OUTPUT);
	pinMode(motor1b_pin, OUTPUT);
	pinMode(motor1speed_pin, OUTPUT);
	pinMode(senseClosed_pin, INPUT);
	pinMode(senseOpened_pin, INPUT);
	pinMode(relay, OUTPUT);
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
		door(false); // close
#ifdef DEBUG
		Serial.println("closed");
#endif
	}
	else if (!isDark && doorState != 1) {
		door(true); // open
#ifdef DEBUG
		Serial.println("opened");
#endif
	}
#ifdef DEBUG
	Serial.println("deep sleep");
#endif
	Sleepy::loseSomeTime(120000); // next check is 2 minutes later
}


bool IsDark()
{
	// light < 80: door should be closed
	// light > 180: door should be opened
	int light = analogRead(light_pin);
	int threshold = 130 + -50 * doorState;
#ifdef DEBUG
	Serial.print(light);
#endif
	if (light < threshold) {
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
#ifdef DEBUG
	Serial.println("DoorMove()");
#endif
	analogWrite(motor1speed_pin, 1200);
	digitalWrite(motor1a_pin, !open);
	digitalWrite(motor1b_pin, open);
	while (digitalRead(sencePin) == LOW && millis() < 30000) {
		delay(20);
	}

	// let move door closing another 1 sec after door sensot alreary reported "door already closed"
	if (!open) {
		delay(1000);
	}

	//DoorStop
#ifdef DEBUG
	Serial.println("DoorStop()");
#endif
	analogWrite(motor1speed_pin, 0);
	digitalWrite(motor1a_pin, LOW);
	digitalWrite(motor1b_pin, LOW);
	digitalWrite(relay, 0);
	doorState = open ? 1 : -1;
}