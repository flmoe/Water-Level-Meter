// Microcontroller ATMEGA 328, 16 MHz, Arduino Bootloader (Arduino Uno)
// Distance sensor: HC-SR04
// Display: 16x1 LCD, HD44780

#include <LiquidCrystal.h>

const int TRIGGER_PIN = 9; 
const int ECHO_PIN = 8; 
const int SWITCH_PIN = 10;
const int BATTERY_PIN = A5;
LiquidCrystal display(2,3,4,5,6,7);

const long CALIBRATION = 0;

// distance between lid and overflow pipe, maximum fill level
const int DISTANCE_MIN = 11;

// distance between lid and bottom, minimum fill level
const int DISTANCE_MAX = 129;

void setup() {
  display.begin(16, 2);
  pinMode(TRIGGER_PIN, OUTPUT); 
  pinMode(ECHO_PIN, INPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(BATTERY_PIN, INPUT);
}

const int N_MEASUREMENTS = 5;
int state = 0; // 0: Display distance, 1: Display volume(liter) and fill level(percent), 2: Display battery voltage
bool switchStateOld = true;
unsigned long lastMeasurement = millis();
long distance;

void loop() {
  // button
  bool switchStateNow = digitalRead(SWITCH_PIN);
  if (switchStateNow != switchStateOld && !switchStateNow) {
    state++;
    if (state > 2) state = 0;
  }
  switchStateOld = switchStateNow;

  // measure battery voltage
  int voltage = map(analogRead(BATTERY_PIN), 0, 1023, 0, 5160);
  bool charge = voltage < 3000;

  // measure distance
  if (millis() - lastMeasurement > 700) {
    long measurements[N_MEASUREMENTS];
    for (int i = 0; i < N_MEASUREMENTS; i++) {
       measurements[i] = measureDistance();
       delay(50);
    }
    distance = median(measurements, N_MEASUREMENTS) + CALIBRATION;
    lastMeasurement = millis();
  }

  // update display
  switch (state) {
    case 0:
        printDistance(distance, charge);
        break;
    case 1:
        printLiter(distance, charge);
        break;
    case 2:
        printBattery(voltage);
        break;    
  }

  delay(60);
}

void printDistance(long distance, bool charge) {
  display.clear(); 
  if (distance >= 500 || distance <= 0) {
    display.print("kein Mes");
    display.setCursor(0, 1);
    display.print("swert");
  }
  else {
    display.print(distance);
    display.print(" cm");
  }
  if (charge) {
    display.setCursor(7, 1);
    display.print("L");
  }
}

void printLiter(long distance, bool charge) {
  display.clear();
  int waterLevel = DISTANCE_MAX - distance;
  int percent = (waterLevel * 100) / (DISTANCE_MAX - DISTANCE_MIN);
  int liter = ((long)percent * 893) / 100;
  display.print(liter);
  display.print(" ltr");
  display.setCursor(0, 1);
  display.print(percent);
  display.print("%");
  if (charge) {
    display.setCursor(7, 1);
    display.print("L");
  }
}

void printBattery(int voltage) {
  display.clear();
  display.print("B: ");
  display.print(voltage);
  display.setCursor(0, 1);
  display.print("mV");
}

long measureDistance() {
  digitalWrite(TRIGGER_PIN, LOW); 
  delay(5); 
  digitalWrite(TRIGGER_PIN, HIGH); 
  delay(10);
  digitalWrite(TRIGGER_PIN, LOW);
  long pulseDuration = pulseIn(ECHO_PIN, HIGH); 
  long distance = (pulseDuration / 2) * 0.03432;  
  return distance;
}

long median(long x[], int n) {
  // simple sort
  for (int i = 0; i < n; i++) {
    for (int j = i; j < n; j++) {
      if (x[j] < x[i]) {
        long tmp = x[i];
        x[i] = x[j];
        x[j] = tmp;  
      }
    }
  }
  return x[n / 2];
}
