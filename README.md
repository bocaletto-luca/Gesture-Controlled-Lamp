# Gesture-Controlled Lamp with BLE Feedback - Arduino
#### Author: Bocaletto Luca

**Files**  
- `README.md`  
- `GestureLamp.ino`

## 1. Concept  
Use an ESP32 + MPU-6050 IMU to control a lamp by hand gestures:  
- **Tilt up/down** → increase/decrease brightness  
- **Shake (quick jerk)** → toggle on/off  
Every brightness change is advertised over BLE so your phone can display real-time status.

## 2. Bill of Materials  
- ESP32 Dev Board (e.g. NodeMCU-32S)  
- MPU-6050 6-axis IMU breakout  
- N-channel MOSFET (IRLZ44N or similar)  
- Lamp or 12 V LED strip  
- 12 V DC power supply (for lamp)  
- 5 V USB supply (for ESP32 & MPU-6050)  
- 10 kΩ resistor (MOSFET gate pulldown)  
- (optional) 100 Ω resistor in series with MOSFET gate  
- Breadboard & jumper wires  

## 3. Wiring Diagram

    MPU-6050         ESP32
    ┌─────────┐     ┌────────┐
    │ VCC  ──► 5V   │ VIN    │
    │ GND  ──► GND  │ GND    │
    │ SDA  ──► 21   │ (I2C)  │
    │ SCL  ──► 22   │ (I2C)  │
    └─────────┘     └────────┘

    Lamp Strip (12 V)      ESP32 + MOSFET
    ┌─────────────┐        ┌─────────┐
    │ +12 V ───► +12 V     │         │
    │ –     ───► Drain    │ IRLZ44N │
    └─────────────┘        │ Gate◄─┬─► GPIO 23
                           │ Src ─►└─ GND
                           └─────────┘
    • Put a 10 kΩ between Gate and GND.  
    • (Optional) 100 Ω series on Gate for noise suppression.

## 4. Software Setup  
1. In Arduino IDE add **ESP32** board definitions.  
2. Install libraries via Library Manager:  
   - `MPU6050` (I2Cdevlib by Jeff Rowberg)  
   - `BLEDevice.h` (built-in with ESP32 support)  
3. Save this folder as `GestureLamp/` containing:
   - `README.md`
   - `GestureLamp.ino`
4. Select **NodeMCU-32S** (or your ESP32) and correct COM port.

## 5. Usage  
1. Power the ESP32 via USB and lamp via 12 V supply.  
2. On boot the ESP starts advertising BLE service “GestureLamp”.  
3. Pair from a generic BLE app (nRF Connect, LightBlue).  
4. Subscribe to the Brightness Characteristic.  
5. **Make gestures**:  
   - Tilt device up/down → lamp brightens/dims by 5%.  
   - Shake device (fast jerk) → toggle lamp ON/OFF.  
6. Watch lamp change and BLE notifications update your phone in real time.

---

Say **continua** to move on to project #9!  
