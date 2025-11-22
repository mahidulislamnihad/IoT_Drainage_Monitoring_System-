#define BLYNK_TEMPLATE_ID "TMPL6BpZ6Au5d"
#define BLYNK_TEMPLATE_NAME "Drain System"
#define BLYNK_AUTH_TOKEN "Q2hgH5SPbR5CCoZW6jHwMXAwPQKmnuxN"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

// WiFi credentials
char ssid[] = "Sophie";
char pass[] = "11223344";

// Pin definitions
#define DRAIN1_PIN 32
#define DRAIN2_PIN 33
#define DRAIN3_PIN 13
#define RAIN_SENSOR_PIN 26
#define GAS_SENSOR_PIN 34
#define FLOW_SENSOR_PIN 35
#define GATE1_SERVO_PIN 27
#define GATE2_SERVO_PIN 14

// Thresholds 
const int WATER_THRESHOLD = 30;
const int GAS_THRESHOLD = 500;
const float FLOW_THRESHOLD = 0.05;

// Servos
Servo gate1Servo;
Servo gate2Servo;

// State variables
bool gate1Closed = false;
bool gate2Closed = false;

// Blynk auth token
char auth[] = BLYNK_AUTH_TOKEN;

void setup() {
  Serial.begin(9600);
  delay(100);

  // Initialize Blynk
  Blynk.begin(auth, ssid, pass);

  // GPIOs as input
  pinMode(DRAIN1_PIN, INPUT);
  pinMode(DRAIN2_PIN, INPUT);
  pinMode(DRAIN3_PIN, INPUT);
  pinMode(RAIN_SENSOR_PIN, INPUT);
  pinMode(GAS_SENSOR_PIN, INPUT);
  pinMode(FLOW_SENSOR_PIN, INPUT);

  //  servos
  gate1Servo.attach(GATE1_SERVO_PIN);
  gate2Servo.attach(GATE2_SERVO_PIN);

  // servo positions (open)
  gate1Servo.write(90);
  gate2Servo.write(90);

  Serial.println("System Initialized.");
}

void loop() {
  Blynk.run();

  //  SENSOR READINGS 
  int drain1Level = analogRead(DRAIN1_PIN);
  int drain2Level = analogRead(DRAIN2_PIN);
  int drain3Level = analogRead(DRAIN3_PIN);
  int rainLevel   = analogRead(RAIN_SENSOR_PIN);
  int gasLevel    = analogRead(GAS_SENSOR_PIN);
  float flowRate  = readFlowSensor();  // simulated flow sensor

  // READINGS print serial monitor
  Serial.println("----- Sensor Readings -----");
  Serial.print("Drain1 Level: "); Serial.println(drain1Level);
  Serial.print("Drain2 Level: "); Serial.println(drain2Level);
  Serial.print("Drain3 Level: "); Serial.println(drain3Level);
  Serial.print("Rain Level: "); Serial.println(rainLevel);
  Serial.print("Gas Level: "); Serial.println(gasLevel);
  Serial.print("Flow Rate: "); Serial.print(flowRate); Serial.println(" L/s");

  Serial.println("Drain 3 Monitoring");
  if (drain3Level > WATER_THRESHOLD) {
    Serial.println("Warning: Drain 3 Water Level Exceeded Threshold!");
  } else {
    Serial.println("Drain 3 Water Level is Normal.");
  }
  Serial.println("................");

  // ------- BLYNK UPDATES -------
  Blynk.virtualWrite(V1, drain1Level);
  Blynk.virtualWrite(V2, drain2Level);
  Blynk.virtualWrite(V3, drain3Level);
  Blynk.virtualWrite(V4, rainLevel);
  Blynk.virtualWrite(V5, gasLevel);
  Blynk.virtualWrite(V6, flowRate);
  Blynk.virtualWrite(V7, gate1Closed ? "Closed" : "Open");
  Blynk.virtualWrite(V8, gate2Closed ? "Closed" : "Open");

  // ------- MAIN LOGIC (UNCHANGED) -------
  if (drain3Level > WATER_THRESHOLD && drain2Level < WATER_THRESHOLD) {
    if (!gate2Closed) {
      gate2Servo.write(0);
      gate2Closed = true;
      Blynk.logEvent("drain3_overflow", "Gate2 Closed");
      Serial.println("Action: Gate2 Closed (Drain3 Overflow)");
    }
  } else if (drain3Level < WATER_THRESHOLD) {
    gate2Servo.write(90);
    gate2Closed = false;
    Serial.println("Action: Gate2 Opened (Drain3 Normal)");
  }

  if (drain2Level > WATER_THRESHOLD && drain1Level < WATER_THRESHOLD) {
    if (!gate1Closed) {
      gate1Servo.write(0);
      gate1Closed = true;
      Blynk.logEvent("drain2_overflow", "Gate1 Closed");
      Serial.println("Action: Gate1 Closed (Drain2 Overflow)");
    }
  } else if (drain2Level < WATER_THRESHOLD) {
    gate1Servo.write(90);
    gate1Closed = false;
    Serial.println("Action: Gate1 Opened (Drain2 Normal)");
  }

  if (drain1Level > WATER_THRESHOLD) {
    gate1Servo.write(90);
    gate2Servo.write(90);
    gate1Closed = false;
    gate2Closed = false;
    Blynk.logEvent("drain1_overflow", "All Gates Open");
    Serial.println("Action: All Gates Open (Drain1 Overflow)");
  }

  if (rainLevel > 500) {
    if (flowRate < FLOW_THRESHOLD) {
      Blynk.logEvent("low_flow", "Cleaning Needed: Flow below threshold during rain");
      Serial.println("Warning: Low Flow during Rain (Cleaning Needed)");
    }
  }

  if (gasLevel > GAS_THRESHOLD) {
    Blynk.logEvent("high_gas", "Clogging Detected: Cleaning Needed");
    Serial.println("Warning: High Gas Level Detected (Clogging Possible)");
  }

  delay(500);
}

float readFlowSensor() {
  //sensor reading is not correct (may be malfunction) due to time limitation taking random value to showcase the prototype and project plan  
  float value = random(10, 100) / 100.0;  // 0.10 to 1.00 L/s 
  return value;
}
