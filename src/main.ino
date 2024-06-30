#define BLYNK_TEMPLATE_ID "TMPL66ZYB08r0"
#define BLYNK_TEMPLATE_NAME "UAS Automatic Water Level"
#define BLYNK_AUTH_TOKEN "FPjRr9T3GpobEoYsrhcCaG4yHBM7cXyM"

#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Blynk Auth Token
char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "Wokwi-GUEST"; 
char pass[] = ""; //password 

LiquidCrystal lcd(0, 4, 14, 27, 26, 25); // Update pins for ESP32

long duration, inches;
int set_val, percentage;
bool pump = false;
bool autoMode = true;

BlynkTimer timer;

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.print("WATER LEVEL:");
  lcd.setCursor(0, 1); 
  lcd.print("PUMP:OFF MANUAL");

  // Initialize pins
  pinMode(5, OUTPUT); // Trigger pin for Ultrasonic sensor
  pinMode(18, INPUT); // Echo pin for Ultrasonic sensor
  pinMode(19, OUTPUT); // Relay control pin

  // Initialize EEPROM
  EEPROM.begin(512);
  set_val = EEPROM.read(0);
  if (set_val > 150) set_val = 150;

  // Initialize WiFi and Blynk
  Blynk.begin(auth, ssid, pass);

  // Set up a function to be called every second
  timer.setInterval(1000L, readSensor);
}

void loop() {
  Blynk.run(); // Run Blynk
  timer.run(); // Run BlynkTimer
}

void readSensor() {
  // Trigger the ultrasonic sensor
  digitalWrite(5, LOW);
  delayMicroseconds(2);
  digitalWrite(5, HIGH);
  delayMicroseconds(10);
  digitalWrite(5, LOW);
  duration = pulseIn(18, HIGH);
  inches = microsecondsToInches(duration);

  percentage = (set_val - inches) * 100 / set_val;

  // Update LCD display
  lcd.setCursor(12, 0);
  if (percentage < 0) percentage = 0;
  lcd.print(percentage);
  lcd.print("%   ");
  
  // Check if pump should be turned on/off based on water level and mode
  if (autoMode) {
    if (percentage < 80) {
      digitalWrite(19, HIGH); // Turn on pump
      pump = true; // Update pump state
    } else {
      digitalWrite(19, LOW); // Turn off pump
      pump = false; // Update pump state
    }
  } else {
    digitalWrite(19, pump);
  }
  
  // Update LCD display for pump state and mode
  lcd.setCursor(5, 1);
  if (pump) {
    lcd.print("ON ");
  } else {
    lcd.print("OFF");
  }

  lcd.setCursor(9, 1);
  if (autoMode) {
    lcd.print("AUTO  ");
  } else {
    lcd.print("MANUAL");
  }

  // Send data to Blynk
  Blynk.virtualWrite(V0, percentage); // Update percentage on Blynk
  Blynk.virtualWrite(V1, pump);       // Update pump state on Blynk
  Blynk.virtualWrite(V2, autoMode); // Update mode on Blynk
}

long microsecondsToInches(long microseconds) {
  return microseconds / 74 / 2;
}

// Blynk widget functions
BLYNK_WRITE(V1) {
  // Function to receive pump state from Blynk app
  pump = param.asInt();
}

BLYNK_WRITE(V2) {
  // Function to receive mode (auto/manual) from Blynk app
  autoMode = param.asInt();
}
