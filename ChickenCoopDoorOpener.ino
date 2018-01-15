/*
used:
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
	// light < 100: door should be closed
	// light > 200: door should be opened
	int light = analogRead(light_pin);
	int threshold = 100 + -20 * doorState;
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
	if (!open) {
		// when door should be closed - delay for 30 minutes
		Sleepy::loseSomeTime(1800000); // 30 * 60 * 1000 
	}
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