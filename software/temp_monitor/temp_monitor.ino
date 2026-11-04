#include <Wire.h>

const int S0 = 2;
const int S1 = 3;
const int S2 = 4;
const int S3 = 5;
const int MUX_EN = 6; // Active low
const int LED = 7; // Fault led
const int MUX_SIG = A0;

const int I2C_ADDRESS = 0x48; // Where the temps get sent
const float FAULT_TEMP = 60.0; // LED blinks above this temperature

float temps[16]; // stores all 16 temperatures

// Placeholders based on 0.6 V at 25 C and -2 mV/C, replace after testing
const float T1 = 25.0;
const float V1 = 0.600;
const float T2 = 75.0;
const float V2 = 0.500;

// Convert voltage to temp using linear interpolation between the two points
float voltageToTemp(float voltage) {
  return T1 + (voltage - V1) * (T2 - T1) / (V2 - V1);
}

void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(MUX_EN, OUTPUT);
  pinMode(LED, OUTPUT);

  digitalWrite(MUX_EN, LOW); // turn on the mux

  Wire.begin();
  Serial.begin(9600);
}

void loop() {
  float maxTemp = -1000;

  for (int ch = 0; ch < 16; ch++) {
    // Pick the channel on the mux
    digitalWrite(S0, ch & 1);
    digitalWrite(S1, (ch >> 1) & 1);
    digitalWrite(S2, (ch >> 2) & 1);
    digitalWrite(S3, (ch >> 3) & 1);
    delay(1);

    // Read the sensor and convert to voltage with the ADC
    int raw = analogRead(MUX_SIG);
    float voltage = raw * 5.0 / 1023.0;

    // volts to temp
    temps[ch] = voltageToTemp(voltage);

    // Print voltage and temp for testing
    Serial.print("CH");
    Serial.print(ch);
    Serial.print(": ");
    Serial.print(voltage, 3);
    Serial.print(" V, ");
    Serial.print(temps[ch], 1);
    Serial.println(" C");

    // Keep the highest temperature
    if (temps[ch] > maxTemp) {
      maxTemp = temps[ch];
    }
  }

  // send max first then all 16
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write((int)maxTemp);
  for (int ch = 0; ch < 16; ch++) {
    Wire.write((int)temps[ch]);
  }
  Wire.endTransmission();

  // Blink red LED if any sensor is above 60 C, otherwise keep it off
  if (maxTemp > FAULT_TEMP) {
    digitalWrite(LED, !digitalRead(LED));
  } else {
    digitalWrite(LED, LOW);
  }

  delay(500);
}