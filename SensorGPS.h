/*
  ============================================================
  SensorGPS.h - GPS (TinyGPS++) untuk lokasi titik pemantauan
  Non-blocking. Dianggap UNHEALTHY jika tidak ada karakter NMEA
  masuk sama sekali dalam GPS_STALE_TIMEOUT_MS (indikasi wiring
  atau baud rate salah, bukan sekadar belum fix satelit).
  ============================================================
*/
#ifndef SENSOR_GPS_H
#define SENSOR_GPS_H

#include <Arduino.h>
#include <TinyGPS++.h>
#include "Config.h"

class SensorGPS {
  public:
    void begin();
    void update();          // non-blocking, panggil tiap loop()
    bool isHealthy();        // false jika tidak ada data masuk sama sekali
    bool hasFix();           // true jika lokasi valid

    double getLat();
    double getLng();
    uint32_t getSatellites();

  private:
    TinyGPSPlus _gps;
    HardwareSerial _serial{1};
    unsigned long _lastCharAt = 0;
    unsigned long _lastCharsProcessed = 0;
};

#endif // SENSOR_GPS_H
