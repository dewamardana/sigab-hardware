/*
  ============================================================
  SensorLidar.h - TF-Luna LiDAR (I2C) - dipakai untuk ketinggian air
  Non-blocking, dengan pelacakan kegagalan beruntun (fail count).
  Jika gagal terus-menerus melebihi TFLUNA_MAX_FAIL_COUNT, sensor
  ditandai UNHEALTHY dan sistem akan mencoba re-init berkala
  (bukan while(true) yang mem-block seluruh program).
  ============================================================
*/
#ifndef SENSOR_LIDAR_H
#define SENSOR_LIDAR_H

#include <Arduino.h>
#include <Wire.h>
#include "Config.h"

class SensorLidar {
  public:
    void begin();
    void update();            // panggil tiap loop(), non-blocking
    bool isHealthy();

    uint16_t getDistanceCM();
    uint16_t getStrength();
    float    getTemperatureC();

  private:
    bool _healthy = false;
    unsigned long _lastReadAt = 0;
    unsigned long _lastRetryAt = 0;
    uint8_t _failCount = 0;

    uint16_t _distanceCM = 0;
    uint16_t _strength = 0;
    float    _temperatureC = 0.0f;

    bool readOnce();          // 1x request+parse, return false kalau gagal/timeout
    void markFailure(const char* reason);
    void markSuccess();
};

#endif // SENSOR_LIDAR_H
