#include "SensorRain.h"
#include "Logger.h"

namespace RainISR {
  volatile unsigned long tipCount = 0;
  volatile unsigned long lastTipMicros = 0;

  void IRAM_ATTR onTip()
  {
    unsigned long now = micros();
    if (now - lastTipMicros > RAIN_DEBOUNCE_US)
    {
      tipCount++;
      lastTipMicros = now;
    }
  }
}

void SensorRain::begin()
{
  pinMode(PIN_RAIN_INTERRUPT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_RAIN_INTERRUPT), RainISR::onTip, FALLING);
  _windowStart = millis();
  logInfo("RAIN", "Rain gauge siap. Jendela: %lu detik", RAIN_WINDOW_MS / 1000);
}

void SensorRain::update()
{
  unsigned long now = millis();
  unsigned long elapsed = now - _windowStart;

  if (elapsed >= RAIN_WINDOW_MS)
  {
    noInterrupts();
    unsigned long tips = RainISR::tipCount;
    RainISR::tipCount = 0;
    interrupts();

    float rainfallMM = tips * RAIN_MM_PER_TIP;
    float hours = elapsed / 3600000.0f;
    _intensityMMh = rainfallMM / hours;
    _category = kategoriDari(_intensityMMh);
    _lastTipCount = tips;
    _totalMM += rainfallMM;

    logDebug("RAIN", "Tip=%lu, intensitas=%.2f mm/jam, kategori=%s",
             tips, _intensityMMh, _category.c_str());

    _windowStart = now;
  }
}

bool SensorRain::isHealthy()
{
  // Sensor tip bucket pasif: dianggap sehat selama tidak ada indikasi hardware lain.
  // Bisa dikembangkan misalnya deteksi tip yang mustahil tinggi (short circuit).
  return true;
}

float SensorRain::getIntensityMMh() { return _intensityMMh; }
String SensorRain::getCategory()    { return _category; }
unsigned long SensorRain::getLastTipCount() { return _lastTipCount; }
float SensorRain::getTotalMM()      { return _totalMM; }
void SensorRain::resetTotalMM()     { _totalMM = 0.0f; }

String SensorRain::kategoriDari(float mmPerJam)
{
  if (mmPerJam <= 0.0f)   return "Tidak ada hujan";
  if (mmPerJam < 2.5f)    return "Hujan Sangat Ringan";
  if (mmPerJam < 10.0f)   return "Hujan Ringan";
  if (mmPerJam < 20.0f)   return "Hujan Sedang";
  if (mmPerJam < 50.0f)   return "Hujan Lebat";
  if (mmPerJam < 100.0f)  return "Hujan Sangat Lebat";
  return "Hujan Ekstrem";
}
