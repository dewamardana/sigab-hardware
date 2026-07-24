#include "SensorGPS.h"
#include "Logger.h"

void SensorGPS::begin()
{
  _serial.begin(GPS_BAUD, SERIAL_8N1, GPS_RXD2, GPS_TXD2);
  _lastCharAt = millis();
  logInfo("GPS", "Modul GPS diinisialisasi pada baud %d", GPS_BAUD);
}

void SensorGPS::update()
{
  bool gotChar = false;
  while (_serial.available() > 0)
  {
    _gps.encode(_serial.read());
    gotChar = true;
  }

  if (gotChar)
  {
    _lastCharAt = millis();
  }

  // Peringatan diagnostik: kalau charsProcessed tidak bertambah dalam waktu lama,
  // biasanya wiring/baud rate bermasalah, bukan soal sinyal satelit.
  if (_gps.charsProcessed() != _lastCharsProcessed)
  {
    _lastCharsProcessed = _gps.charsProcessed();
  }
}

bool SensorGPS::isHealthy()
{
  return (millis() - _lastCharAt) < GPS_STALE_TIMEOUT_MS;
}

bool SensorGPS::hasFix()      { return _gps.location.isValid(); }
double SensorGPS::getLat()    { return _gps.location.lat(); }
double SensorGPS::getLng()    { return _gps.location.lng(); }
uint32_t SensorGPS::getSatellites()
{
  return _gps.satellites.isValid() ? _gps.satellites.value() : 0;
}
