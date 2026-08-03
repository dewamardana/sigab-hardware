#include "MosfetSwitch.h"

MosfetSwitch::MosfetSwitch(uint8_t pin, bool activeHigh)
  : _pin(pin), _activeHigh(activeHigh) {}

void MosfetSwitch::begin()
{
  pinMode(_pin, OUTPUT);
  turnOff(); // fail-safe: pastikan OFF di awal, sebelum kondisi apa pun dievaluasi
}

void MosfetSwitch::turnOn()  { set(true); }
void MosfetSwitch::turnOff() { set(false); }

void MosfetSwitch::set(bool on)
{
  _isOn = on;
  bool pinHigh = _activeHigh ? on : !on;
  digitalWrite(_pin, pinHigh ? HIGH : LOW);
}

bool MosfetSwitch::isOn() { return _isOn; }