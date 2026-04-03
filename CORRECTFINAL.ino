#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// -------- Pin Definitions --------
#define PH_PIN        1
#define TURBIDITY_PIN 4
#define ONE_WIRE_BUS  6   // DS18B20

// -------- pH Calibration Values --------
#define PH7_VOLTAGE  1.50
#define PH4_VOLTAGE  2.03

// -------- LCD --------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// -------- OneWire Setup --------
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== River Monitoring System ===");

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  tempSensor.begin();

  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MICROPLASTIC");
  lcd.setCursor(0, 1);
  lcd.print("RISK DETECTION");
  delay(3000);
  lcd.clear();
}

// -------- Read Temperature --------
float readTemperature() {
  tempSensor.requestTemperatures();
  return tempSensor.getTempCByIndex(0);
}

// -------- Read pH --------
float readPH() {
  long total = 0;

  for (int i = 0; i < 20; i++) {
    total += analogRead(PH_PIN);
    delay(10);
  }

  float avgRaw = total / 20.0;
  float voltage = avgRaw * (3.3 / 4095.0);

  float slope = (PH7_VOLTAGE - PH4_VOLTAGE) / (7.0 - 4.0);
  float ph = 7.0 + (PH7_VOLTAGE - voltage) / slope;

  if (ph < 0.0) ph = 0.0;
  if (ph > 14.0) ph = 14.0;

  return ph;
}

// -------- Read Turbidity --------
float readTurbidity() {
  long total = 0;

  for (int i = 0; i < 20; i++) {
    total += analogRead(TURBIDITY_PIN);
    delay(10);
  }

  float avgRaw = total / 20.0;
  float voltage = avgRaw * (3.3 / 4095.0);

  float turbidityPercent = (1 - (voltage / 3.3)) * 100.0;

  if (turbidityPercent < 0) turbidityPercent = 0;
  if (turbidityPercent > 100) turbidityPercent = 100;

  return turbidityPercent;
}

void loop() {

  float temperature = readTemperature();
  float ph = readPH();
  float turbidity = readTurbidity();

  Serial.println("----- Sensor Readings -----");

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("pH Value: ");
  Serial.println(ph, 2);

  Serial.print("Turbidity: ");
  Serial.print(turbidity, 1);
  Serial.println(" %");

  Serial.println("---------------------------\n");

  // -------- Microplastic Risk Logic (Only Once) --------
  String riskLevel;

  if (turbidity > 70) {
    riskLevel = "HIGH";
  }
  else if (turbidity > 40) {
    riskLevel = "MEDIUM";
  }
  else {
    riskLevel = "LOW";
  }

  // -------- LCD TEMPLATE DISPLAY --------

  // 1️⃣ Temperature Screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TEMPERATURE");
  lcd.setCursor(0, 1);
  lcd.print(temperature, 1);
  lcd.print(" C");
  delay(2000);

  // 2️⃣ pH Screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PH VALUE");
  lcd.setCursor(0, 1);
  lcd.print(ph, 2);
  delay(2000);

  // 3️⃣ Turbidity Screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TURBIDITY");
  lcd.setCursor(0, 1);
  lcd.print(turbidity, 1);
  lcd.print(" %");
  delay(2000);

  // 4️⃣ Final Risk Screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MICROPLASTIC");
  lcd.setCursor(0, 1);
  lcd.print("RISK: ");
  lcd.print(riskLevel);
  delay(2000);
}