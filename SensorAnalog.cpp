#include "SensorAnalog.h"

void SensorAnalog::begin()
{
  // Tidak perlu pinMode khusus untuk ADC di ESP32; atenuasi diatur global
  // di setup() lewat analogSetAttenuation(ADC_11db).
}

float SensorAnalog::read()
{
  int mV = analogReadMilliVolts(_pin);
  return _offset + (mV / 3300.0f) * _skala;
}

bool SensorAnalog::isHealthy()
{
  // ADC internal ESP32 tidak melaporkan status gagal komunikasi seperti sensor I2C/UART.
  return true;
}
