#include "SensorFloatSwitch.h"
#include "Logger.h"

void SensorFloatSwitch::begin()
{
  pinMode(PIN_FLOAT_SWITCH, INPUT_PULLUP);
  _lastRawState = digitalRead(PIN_FLOAT_SWITCH);
  _state = (_lastRawState == LOW); // LOW = air tinggi (konfigurasi NO)
  logInfo("FLOAT", "Float switch siap. Status awal: %s", _state ? "TINGGI" : "NORMAL");
}

void SensorFloatSwitch::update()
{
  bool raw = digitalRead(PIN_FLOAT_SWITCH);
  unsigned long now = millis();

  if (raw != _lastRawState)
  {
    _lastChangeAt = now;
    _lastRawState = raw;
  }

  if (now - _lastChangeAt >= FLOAT_DEBOUNCE_MS)
  {
    bool newState = (raw == LOW);
    if (newState != _state)
    {
      _state = newState;
      if (_state)
      {
        logWarn("FLOAT", "Level air TINGGI terdeteksi!");
      }
      else
      {
        logInfo("FLOAT", "Level air kembali normal.");
      }
    }
  }
}

bool SensorFloatSwitch::isHealthy()
{
  // Sensor mekanis pasif: tidak ada mode gagal komunikasi untuk dideteksi di sini.
  return true;
}

bool SensorFloatSwitch::isWaterHigh() { return _state; }
