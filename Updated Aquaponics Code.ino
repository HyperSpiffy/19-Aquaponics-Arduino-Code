/* * Aquaponics Simulation System
 * ----------------------------
 * Monitors Temperature, pH, and Water Levels.
 * * External Libraries:
 * - Adafruit_LiquidCrystal (v2.0.2): https://github.com/adafruit/Adafruit_LiquidCrystal
 */


#include <Adafruit_LiquidCrystal.h>

// --- Pin Definitions (Consistent Naming & Standard) ---
#define PIN_DIAL    A0 
#define PIN_PH      A1
#define PIN_BUTTON  2
#define PIN_TRIG    6
#define PIN_ECHO    7
#define PIN_MOTOR   13
#define PIN_RED     10
#define PIN_GREEN   12
#define PIN_BLUE    11
#define PIN_LED_WARN 5

// --- Configuration & Constants ---
const int REFRESH_INTERVAL = 200; // Screen update rate in ms
const int DEBOUNCE_DELAY   = 50;  // Button debounce in ms

// --- Global Variables ---
Adafruit_LiquidCrystal lcd(0); 

int menuPage = 0;        
int lastButtonState = LOW;
unsigned long lastRefreshTime = 0;

// --- Modular Functions ---

/**
 * Reads the analog temperature sensor and converts to Celsius.
 */
float readTemperature() {
  int val = analogRead(PIN_DIAL);
  float voltage = val * (5.0 / 1023.0);
  return (voltage - 0.5) * 100.0;
}

/**
 * Reads the pH potentiometer and maps it to a 0-14 scale.
 */
float readPH() {
  int val = analogRead(PIN_PH);
  // Adjusted mapping: 21.22/1023 based on original calibration
  return val * (21.22 / 1023.0); 
}

/**
 * Measures distance using the HC-SR04 Ultrasonic sensor.
 */
float readDistance() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long duration = pulseIn(PIN_ECHO, HIGH, 26000); 
  if (duration == 0) return 999; // Return high value if out of range
  return duration / 58.0;
}

/**
 * Controls the RGB LED based on pH levels.
 */
void updateStatusLEDs(float ph) {
  if (ph >= 6.5 && ph <= 8.5) {
    digitalWrite(PIN_GREEN, HIGH);
    digitalWrite(PIN_RED, LOW);
    digitalWrite(PIN_BLUE, LOW);
  } else if (ph < 6.5) {
    digitalWrite(PIN_RED, HIGH);
    digitalWrite(PIN_GREEN, LOW);
    digitalWrite(PIN_BLUE, LOW);
  } else {
    digitalWrite(PIN_BLUE, HIGH);
    digitalWrite(PIN_RED, LOW);
    digitalWrite(PIN_GREEN, LOW);
  }
}

/**
 * Handles the LCD display logic based on the current menu page.
 */
void updateDisplay(float temp, float ph, float dist) {
  if (menuPage == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temp, 1);
    lcd.print("C   "); 
    
    lcd.setCursor(0, 1);
    lcd.print("PH: ");
    lcd.print(ph, 1);
    lcd.print("    ");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Dist: ");
    lcd.print(dist, 1);
    lcd.print(" cm  ");
    
    lcd.setCursor(0, 1);
    if (dist >= 0 && dist <= 10) {
      lcd.print("Status: Safe  ");
      digitalWrite(PIN_LED_WARN, LOW);
    } else {
      lcd.print("Status: DANGER");
      digitalWrite(PIN_LED_WARN, HIGH);
    }
  }
}

// --- Main Program Setup & Loop ---

void setup() {
  Serial.begin(9600);
  
  lcd.begin(16, 2);
  lcd.setBacklight(1);
  
  pinMode(PIN_DIAL, INPUT);
  pinMode(PIN_PH, INPUT);
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_ECHO, INPUT);
  
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_MOTOR, OUTPUT);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_LED_WARN, OUTPUT);
}

void loop() {
  // 1. Sensor Data Acquisition
  float currentTemp = readTemperature();
  float currentPH   = readPH();
  float currentDist = readDistance();

  // 2. Hardware Control Logic
  // Motor triggers if temp is outside the 21-30C range
  if (currentTemp >= 21 && currentTemp <= 30) {
    digitalWrite(PIN_MOTOR, LOW);
  } else {
    digitalWrite(PIN_MOTOR, HIGH);
  }
  
  updateStatusLEDs(currentPH);

  // 3. User Input (Button Handling)
  int currentButtonState = digitalRead(PIN_BUTTON);
  if (currentButtonState == HIGH && lastButtonState == LOW) {
    menuPage = (menuPage + 1) % 2; // Toggle between 0 and 1
    lcd.clear();
    delay(DEBOUNCE_DELAY); 
  }
  lastButtonState = currentButtonState;

  // 4. Timed Display Update
  if (millis() - lastRefreshTime >= REFRESH_INTERVAL) {
    lastRefreshTime = millis();
    updateDisplay(currentTemp, currentPH, currentDist);
  }
}