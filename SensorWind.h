/*
  ============================================================
  SensorWind.h - Anemometer (Kecepatan Angin)
  Interrupt-driven, dihitung per jendela waktu (default 10 detik).
  ============================================================
*/
#ifndef SENSOR_WIND_H
#define SENSOR_WIND_H

#include <Arduino.h>
#include "Config.h"

class SensorWind {
  public:
    void begin();
    void update();     // non-blocking
    bool isHealthy();

    float getSpeedMS();
    float getSpeedKMH();
    float getRPS();

  private:
    unsigned long _windowStart = 0;
    float _rps = 0.0f;
    float _speedMS = 0.0f;
    float _speedKMH = 0.0f;
};

namespace WindISR {
  extern volatile unsigned long pulseCount;
  extern volatile unsigned long lastPulseMicros;
  void IRAM_ATTR onPulse();
}

#endif // SENSOR_WIND_H
