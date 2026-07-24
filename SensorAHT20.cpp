#include "SensorAHT20.h"
#include "Logger.h"

void SensorAHT20::begin()
{
  Wire.begin();
  if (_aht20.begin())
  {
    _healthy = true;
    _failCount = 0;
    logInfo("AHT20", "Sensor terdeteksi dan siap.");
  }
  else
  {
    _healthy = false;
    logError("AHT20", "Sensor TIDAK terdeteksi. Cek wiring I2C. Akan dicoba lagi berkala.");
  }
  _lastRetryAt = millis();
}

void SensorAHT20::update()
{
  unsigned long now = millis();

  if (!_healthy)
  {
    if (now - _lastRetryAt >= AHT20_RETRY_INTERVAL_MS)
    {
      logWarn("AHT20", "Mencoba re-inisialisasi sensor...");
      begin();
    }
    return;
  }

  if (now - _lastReadAt < AHT20_READ_INTERVAL_MS)
  {
    return;
  }
  _lastReadAt = now;

  float t = _aht20.getTemperature();
  float h = _aht20.getHumidity();

  // Nilai NaN atau di luar rentang wajar menandakan pembacaan gagal
  bool valid = !isnan(t) && !isnan(h) && (h >= 0.0f && h <= 100.0f) && (t > -40.0f && t < 85.0f);

  if (valid)
  {
    _temperatureC = t;
    _humidityRH = h;
    _failCount = 0;
  }
  else
  {
    _failCount++;
    logWarn("AHT20", "Pembacaan tidak valid (%d/%d)", _failCount, AHT20_MAX_FAIL_COUNT);
    if (_failCount >= AHT20_MAX_FAIL_COUNT)
    {
      _healthy = false;
      _lastRetryAt = millis();
      logError("AHT20", "Sensor dianggap UNHEALTHY setelah %d kegagalan beruntun.", _failCount);
    }
  }
}

bool SensorAHT20::isHealthy()        { return _healthy; }
float SensorAHT20::getTemperatureC() { return _temperatureC; }
float SensorAHT20::getHumidityRH()   { return _humidityRH; }
