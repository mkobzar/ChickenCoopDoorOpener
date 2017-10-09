// For L298N Dual H-Bridge Motor Controller
// pro mini is board in use

// Motor 1 // D2, D3, D9 => works with nodemcu
int in1_pin = PCINT19;
int in2_pin = PCINT20;
int enA_pin = PD7; // enA_pin needs to be a PWM
int senseClosed_pin = PD5;
int senseOpened_pin = PD6;
int light_pin = A0;

//bool isClosed = false;
int doorState = -1; // -1 is closed, 0 is unknown, 1 is opened

void setup() {
  Serial.begin(9600);
  pinMode(in1_pin, OUTPUT);
  pinMode(in2_pin, OUTPUT);
  pinMode(enA_pin, OUTPUT);
  pinMode(senseClosed_pin,  INPUT);
  pinMode(senseOpened_pin,  INPUT);

}


void loop() {
  Serial.println("loop begin");
  bool isDark = IsDark();
  if (isDark && !doorState == -1) {
    DoorClose();
    Serial.println("closed");
  }
  else if (!isDark && !doorState == 1) {
    DoorOpen();
    Serial.println("opened");
  } else {
    Serial.println("deep sleep");
    delay(5000);
  }
}


bool IsDark()
{
  int light = analogRead(light_pin);
  int threshold = 400 + 50 * doorState;
    Serial.print(light);
  if (light < threshold ) {
    Serial.println(" = night");
    return true;
  }
  else {
    Serial.println(" = day");
    return false;
  }
}

void DoorOpen()
{
  Serial.println("DoorOpen()");
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
  Serial.println("DoorClose()");
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
  Serial.println("DoorStop()");
  analogWrite(enA_pin, 0);
  digitalWrite(in1_pin, LOW);
  digitalWrite(in2_pin, LOW);
}
