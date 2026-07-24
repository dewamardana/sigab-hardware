#include "SensorWind.h"
#include "Logger.h"

namespace WindISR {
  volatile unsigned long pulseCount = 0;
  volatile unsigned long lastPulseMicros = 0;

  void IRAM_ATTR onPulse()
  {
    unsigned long now = micros();
    if ((long)(now - lastPulseMicros) >= (long)WIND_DEBOUNCE_US)
    {
      pulseCount++;
      lastPulseMicros = now;
    }
  }
}

void SensorWind::begin()
{
  pinMode(PIN_WIND_INTERRUPT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_WIND_INTERRUPT), WindISR::onPulse, RISING);
  _windowStart = millis();
  logInfo("WIND", "Anemometer siap. Jendela: %lu detik", WIND_WINDOW_MS / 1000);
}

void SensorWind::update()
{
  unsigned long now = millis();
  if (now - _windowStart < WIND_WINDOW_MS)
  {
    return;
  }

  // Nonaktifkan interrupt sesaat saat membaca & mereset counter (konsisten dengan kode asli)
  detachInterrupt(digitalPinToInterrupt(PIN_WIND_INTERRUPT));

  noInterrupts();
  unsigned long pulses = WindISR::pulseCount;
  WindISR::pulseCount = 0;
  interrupts();

  _rps = (float)pulses / (WIND_WINDOW_MS / 1000.0f);

  _speedMS = (-0.0181f * (_rps * _rps)) + (1.3859f * _rps) + 1.4055f;
  if (_speedMS <= WIND_MIN_MS)
  {
    _speedMS = 0.0f;
  }
  _speedKMH = _speedMS * 3.6f;

  logDebug("WIND", "RPS=%.2f, Kecepatan=%.2f m/s (%.2f km/jam)", _rps, _speedMS, _speedKMH);

  _windowStart = now;
  attachInterrupt(digitalPinToInterrupt(PIN_WIND_INTERRUPT), WindISR::onPulse, RISING);
}

bool SensorWind::isHealthy()
{
  // Sensor pasif berbasis interrupt; dianggap sehat selama pin terpasang.
  // Bisa ditambah deteksi "macet" jika RPS selalu 0 dalam waktu sangat lama saat cuaca ekstrem, jika diperlukan.
  return true;
}

float SensorWind::getSpeedMS()  { return _speedMS; }
float SensorWind::getSpeedKMH() { return _speedKMH; }
float SensorWind::getRPS()      { return _rps; }
