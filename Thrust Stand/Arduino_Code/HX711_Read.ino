#include "HX711.h"

// Connect HX711 DOUT to pin 2 and SCK to pin 3.
const uint8_t HX711_DOUT_PIN = 2;
const uint8_t HX711_SCK_PIN = 3;
const float CALIBRATION_FORCE_N = 9.80665f;

HX711 loadCell;
bool isTared = false;
bool isCalibrated = false;
volatile bool hx711DataReady = false;

void hx711DataReadyISR() {
  hx711DataReady = true;
}

void setup() {
  Serial.begin(115200);
  loadCell.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  attachInterrupt(digitalPinToInterrupt(HX711_DOUT_PIN), hx711DataReadyISR, FALLING);
  Serial.println("HX711 thrust sensor");
  Serial.println("Remove all force, then send 't' to tare.");
  Serial.println("Apply the calibration force, then send 'c' to calibrate.");
  Serial.println("Change CALIBRATION_FORCE_N to match your calibration force.");
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();

    if (command == 't' || command == 'T') {
      Serial.println("Taring...");
      loadCell.tare();
      isTared = true;
      isCalibrated = false;
      Serial.println("Tare complete. Apply the calibration force and send 'c'.");
    }
    else if (command == 'c' || command == 'C') {
      if (isTared) {
        Serial.println("Calibrating...");
        loadCell.calibrate_scale(CALIBRATION_FORCE_N);
        isCalibrated = true;
        Serial.println("Calibration complete. Readings are in newtons.");
      }
      else {
        Serial.println("Tare first: remove all force and send 't'.");
      }
    }
  }

  // Save the interrupt flag locally so the main loop can decide whether to read.
  bool shouldRead = false;
  noInterrupts();
  // Copy whether the HX711 signaled that a new reading is ready.
  shouldRead = hx711DataReady;
  // Clear the shared flag so a later interrupt can signal the next reading.
  hx711DataReady = false;
  interrupts();

  if (shouldRead && loadCell.is_ready()) {
    if (isCalibrated) {
      Serial.print("force_N,");
      Serial.println(loadCell.get_units(), 3);
    }
    else {
      Serial.print("raw,");
      Serial.println(loadCell.read());
    }
  }
}