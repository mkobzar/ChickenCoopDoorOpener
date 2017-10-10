// for pro mini is board with L298N Dual H-Bridge Motor Controller

#define DEBUG

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
		DoorMove(false);
#ifdef DEBUG
		Serial.println("closed");
#endif
	}
	else if (!isDark && doorState != 1) {
		DoorMove(true);
#ifdef DEBUG
		Serial.println("opened");
#endif
	}
	else {
#ifdef DEBUG
		Serial.println("deep sleep");
#endif
		Sleepy::loseSomeTime(3000);
	}
}


bool IsDark()
{
	// 100 is very dark. at 100 light value door can be closed
	// 300 is dark, at 300 door can be opened
	int light = analogRead(light_pin);
	int threshold = 200 + -100 * doorState;
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

 
void DoorMove(bool open)
{
	reset_millis();
	unsigned long timeStart = millis() + 15000;
	int sencePin = open ? senseOpened_pin : senseClosed_pin;
#ifdef DEBUG
	Serial.println("DoorMove()");
#endif
	analogWrite(motor1speed_pin, 1200);
	digitalWrite(motor1a_pin, !open);
	digitalWrite(motor1b_pin, open);
	while (digitalRead(sencePin) == LOW && millis() < timeStart) {
		delay(20);
	}
	DoorStop();
	doorState = open ? 1 : -1;
}
 
void DoorStop()
{
#ifdef DEBUG
	Serial.println("DoorStop()");
#endif
	analogWrite(motor1speed_pin, 0);
	digitalWrite(motor1a_pin, LOW);
	digitalWrite(motor1b_pin, LOW);
}


void reset_millis()
{
	// http://forum.arduino.cc/index.php?topic=189138.0
	extern volatile unsigned long timer0_millis, timer0_overflow_count;
	noInterrupts();
	timer0_millis = timer0_overflow_count = 0;
	interrupts();
}
