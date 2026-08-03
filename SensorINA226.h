/*
  ============================================================
  SensorINA226.h - Tegangan Baterai via INA226 (I2C)
  Pola SAMA PERSIS dengan SensorAHT20/SensorLidar: update() non-blocking
  dipanggil tiap loop(), interval internal (INA226_READ_INTERVAL_MS)
  menentukan seberapa sering I2C benar-benar diakses, retry otomatis
  berkala kalau sensor unhealthy. TIDAK ADA delay()/polling blocking
  sama sekali - trigger konversi & baca hasilnya dipisah jadi 2 state
  yang dijalankan lintas panggilan update() (lihat State di bawah).
  ============================================================
*/
#ifndef SENSOR_INA226_H
#define SENSOR_INA226_H

#include <Arduino.h>
#include <Wire.h>
#include "Config.h"

class SensorINA226 {
  public:
    void begin();
    void update();           // non-blocking, panggil tiap loop()
    bool isHealthy();

    float getBusVoltageV();  // tegangan baterai terkini (V)

  private:
    enum class State { IDLE, WAITING_CONVERSION };

    bool _healthy = false;
    State _state = State::IDLE;
    unsigned long _lastReadAt = 0;
    unsigned long _lastRetryAt = 0;
    unsigned long _triggeredAt = 0;
    uint8_t _failCount = 0;

    float _busVoltageV = 0.0f;

    bool writeRegister16(uint8_t reg, uint16_t value);
    bool readRegister16(uint8_t reg, uint16_t &value);
    void markFailure(const char* reason);
    void markSuccess();
};

#endif // SENSOR_INA226_H