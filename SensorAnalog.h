/*
  ============================================================
  SensorAnalog.h - Sensor berbasis pembacaan analog (potensiometer)
  Satu class generik dipakai ulang untuk 3 sensor (TMA, kecepatan
  angin, tegangan baterai) supaya tidak ada duplikasi kode -
  masing-masing beda pin & rumus konversi saja, konfigurasi lewat
  konstruktor.
  ============================================================
*/
#ifndef SENSOR_ANALOG_H
#define SENSOR_ANALOG_H

#include <Arduino.h>

class SensorAnalog {
  public:
    // pin       : GPIO ADC
    // skala     : nilai maksimum hasil konversi (pada 3300 mV)
    // offset    : nilai dasar yang ditambahkan (dipakai utk baterai, misal mulai dari 10V)
    SensorAnalog(uint8_t pin, float skala, float offset = 0.0f)
      : _pin(pin), _skala(skala), _offset(offset) {}

    void begin();
    float read();      // baca langsung (dipanggil dari update() sensor lain saat interval tercapai)
    bool isHealthy();   // pembacaan analog internal ESP32 tidak punya mode gagal komunikasi

  private:
    uint8_t _pin;
    float _skala;
    float _offset;
};

#endif // SENSOR_ANALOG_H
