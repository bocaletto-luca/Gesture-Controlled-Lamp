/* File: GestureLamp.ino
   ESP32 + MPU6050 Gesture-Controlled Lamp with BLE Feedback
*/

#include <Wire.h>
#include "MPU6050.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ===== Pin & BLE Definitions =====
const int    MOSFET_PIN      = 23;      // PWM → MOSFET gate
const int    I2C_SDA         = 21;      // MPU6050 SDA
const int    I2C_SCL         = 22;      // MPU6050 SCL

// BLE service/characteristic UUIDs (randomly generated)
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHAR_UUID_BRIGHT    "abcdefab-1234-5678-1234-abcdefabcdef"

MPU6050     imu;
BLECharacteristic* pBrightnessChar;

// ===== Gesture Settings =====
const float   TILT_THRESH     = 20.0;   // deg tilt to step brightness
const float   SHAKE_THRESH    = 1.5;    // g-units to detect shake
const uint8_t BRIGHT_STEP     = 13;     // 5% of 255 ≈ 13
const uint32_t GESTURE_DELAY  = 300;    // ms debounce

// ===== State =====
uint8_t  brightness = 0;    // 0–255
bool     lampOn     = false;
uint32_t lastGesture = 0;

// ===== BLE Callbacks =====
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer*)  { /* no action */ }
  void onDisconnect(BLEServer* pServer) {
    pServer->startAdvertising();
  }
};

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  // Init MPU6050
  imu.initialize();
  if (!imu.testConnection()) {
    Serial.println("MPU6050 error!");
    while (1);
  }

  // Setup PWM on MOSFET_PIN (2 kHz, 8-bit)
  ledcSetup(0, 2000, 8);
  ledcAttachPin(MOSFET_PIN, 0);
  ledcWrite(0, 0);

  // Setup BLE
  BLEDevice::init("GestureLamp");
  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService* pService = pServer->createService(SERVICE_UUID);
  pBrightnessChar = pService->createCharacteristic(
                      CHAR_UUID_BRIGHT,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pBrightnessChar->addDescriptor(new BLE2902());
  pBrightnessChar->setValue(&brightness, 1);
  pService->start();
  pServer->getAdvertising()->start();
  Serial.println("BLE Advertising GestureLamp");
}

void loop() {
  // Read accelerometer (g-units)
  int16_t ax, ay, az;
  imu.getAcceleration(&ax, &ay, &az);
  float xg = ax / 16384.0;
  float yg = ay / 16384.0;
  float zg = az / 16384.0;

  // Compute pitch (tilt forward/back)
  float pitch = atan2(-xg, sqrt(yg*yg + zg*zg)) * 57.3;
  // Compute acceleration magnitude for shake detection
  float accMag = sqrt(xg*xg + yg*yg + zg*zg);

  uint32_t now = millis();
  if (now - lastGesture > GESTURE_DELAY) {
    // Tilt up → increase brightness
    if (pitch >  TILT_THRESH && lampOn) {
      brightness = min(255, brightness + BRIGHT_STEP);
      lastGesture = now;
    }
    // Tilt down → decrease brightness
    else if (pitch < -TILT_THRESH && lampOn) {
      brightness = max(0, brightness - BRIGHT_STEP);
      lastGesture = now;
    }
    // Shake → toggle ON/OFF
    else if (accMag > SHAKE_THRESH) {
      lampOn     = !lampOn;
      brightness = lampOn ? 128 : 0;  // default mid-level
      lastGesture = now;
    }
    // Apply lamp state
    ledcWrite(0, lampOn ? brightness : 0);

    // Notify BLE client
    pBrightnessChar->setValue(&brightness, 1);
    pBrightnessChar->notify();
  }

  delay(20);
}
