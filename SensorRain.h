/*
  ============================================================
  SensorRain.h - Rain Gauge (Tipping Bucket)
  Kategori intensitas real-time (mm/jam), diekstrapolasi dari
  tip dalam jendela berjalan (default 1 menit).
  Referensi kategori: BMKG - klasifikasi curah hujan sesaat.
  ============================================================
*/
#ifndef SENSOR_RAIN_H
#define SENSOR_RAIN_H

#include <Arduino.h>
#include "Config.h"
#include <Wire.h>
#include "RTClib.h"


class SensorRain {
  public:
    void begin();
    void update();                  // panggil tiap loop(), non-blocking
    bool isHealthy();                // sensor tip selalu dianggap sehat (pasif), kecuali dipakai untuk cek pin stuck
    float getIntensityMMh();
    String getCategory();
    unsigned long getLastTipCount(); // jumlah tip di jendela terakhir
    float getTotalMM();              // akumulasi total sejak boot/reset
    void resetTotalMM();             // panggil kalau perlu reset akumulasi (mis. tengah malam)

  private:
    unsigned long _windowStart = 0;
    float _intensityMMh = 0.0f;
    String _category = "Tidak ada hujan";
    unsigned long _lastTipCount = 0;
    float _totalMM = 0.0f;

    static String kategoriDari(float mmPerJam);
};

// ISR harus berada di luar class (batasan Arduino), diekspos lewat namespace kecil
namespace RainISR {
  extern volatile unsigned long tipCount;
  extern volatile unsigned long lastTipMicros;
  void IRAM_ATTR onTip();
}

#endif // SENSOR_RAIN_H
