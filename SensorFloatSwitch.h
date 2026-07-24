/*
  ============================================================
  SensorFloatSwitch.h - Saklar Pelampung (Level Air Kritis)
  Sensor digital sederhana dengan debounce berbasis waktu
  (non-blocking, tanpa delay()).
  ============================================================
*/
#ifndef SENSOR_FLOAT_SWITCH_H
#define SENSOR_FLOAT_SWITCH_H

#include <Arduino.h>
#include "Config.h"

class SensorFloatSwitch {
  public:
    void begin();
    void update();       // non-blocking
    bool isHealthy();
    bool isWaterHigh();  // true jika level air tinggi (kritis)

  private:
    bool _state = false;
    bool _lastRawState = false;
    unsigned long _lastChangeAt = 0;
};

#endif // SENSOR_FLOAT_SWITCH_H
