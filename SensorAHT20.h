/*
  ============================================================
  SensorAHT20.h - Suhu & Kelembapan
  Menggantikan pola `while (aht20.begin() == false) {}` (blocking)
  dengan retry non-blocking berkala, supaya sensor yang belum
  terpasang/putus kabel tidak menghentikan seluruh sistem.
  ============================================================
*/
#ifndef SENSOR_AHT20_H
#define SENSOR_AHT20_H

#include <Arduino.h>
#include <AHT20.h>
#include "Config.h"

class SensorAHT20 {
  public:
    void begin();
    void update();          // non-blocking
    bool isHealthy();

    float getTemperatureC();
    float getHumidityRH();

  private:
    AHT20 _aht20;
    bool _healthy = false;
    unsigned long _lastReadAt = 0;
    unsigned long _lastRetryAt = 0;
    uint8_t _failCount = 0;

    float _temperatureC = 0.0f;
    float _humidityRH = 0.0f;
};

#endif // SENSOR_AHT20_H
