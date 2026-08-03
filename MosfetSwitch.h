/*
  ============================================================
  MosfetSwitch.h - Saklar ON/OFF generik lewat modul MOSFET
  (IRF520 atau modul switching sejenis, 1 pin SIG digital).

  Dipakai untuk BEBAN YANG DISWITCH ON/OFF SAJA (buzzer, lampu flash/
  senter, relai, dll) - BUKAN untuk beban yang butuh PWM/dimming.

  Cara kerja modul IRF520 (breakout umum):
    - SIG   -> disambung ke GPIO ESP32 di bawah (logic 3.3V)
    - VCC   -> 3.3V/5V ESP32 (catu daya logic modul)
    - GND   -> GND ESP32 (WAJIB common ground dgn catu daya beban)
    - V+    -> (+) catu daya beban (mis. 12V), TERPISAH dari ESP32
    - V-    -> (-) catu daya beban, disatukan ke GND ESP32
    - OUT   -> ke salah satu terminal beban (mis. buzzer), terminal
               beban satunya ke V+
  SIG HIGH umumnya = MOSFET ON (beban menyala) - lihat parameter
  activeHigh di constructor kalau modul kamu terbalik (SIG LOW = ON).
  ============================================================
*/
#ifndef MOSFET_SWITCH_H
#define MOSFET_SWITCH_H

#include <Arduino.h>

class MosfetSwitch {
  public:
    MosfetSwitch(uint8_t pin, bool activeHigh = true);

    void begin();      // set pinMode OUTPUT & pastikan OFF (fail-safe di boot)
    void turnOn();
    void turnOff();
    void set(bool on);  // set(true)=ON, set(false)=OFF - praktis utk sinkron ke kondisi sensor
    bool isOn();

  private:
    uint8_t _pin;
    bool _activeHigh;
    bool _isOn = false;
};

#endif // MOSFET_SWITCH_H