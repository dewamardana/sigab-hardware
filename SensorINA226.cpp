#include "SensorINA226.h"
#include "Logger.h"

// ---------------- Alamat register INA226 (datasheet Table 7-1) ----------------
static const uint8_t REG_CONFIG          = 0x00;
static const uint8_t REG_BUS_VOLTAGE     = 0x02;
static const uint8_t REG_MANUFACTURER_ID = 0xFE;

static const uint16_t MANUFACTURER_ID_TI = 0x5449; // "TI", datasheet 7.1.9

// POR default Configuration Register = 4127h (AVG=1x, VBUSCT=1.1ms,
// VSHCT=1.1ms, MODE=111 continuous). Kita ganti HANYA bit MODE (D2:D0)
// jadi '010' = Bus Voltage, Triggered - satu kali konversi bus voltage
// saja tiap dipicu (shunt voltage sama sekali tidak diukur/dikalibrasi).
static const uint16_t CONFIG_POR_DEFAULT         = 0x4127;
static const uint16_t MODE_BITMASK               = 0x0007;
static const uint16_t MODE_BUS_VOLTAGE_TRIGGERED = 0x0002;
static const uint16_t CONFIG_TRIGGER_BUS_VOLTAGE =
    (CONFIG_POR_DEFAULT & ~MODE_BITMASK) | MODE_BUS_VOLTAGE_TRIGGERED; // = 0x4122

static const float BUS_VOLTAGE_LSB_V = 0.00125f; // 1.25 mV/bit, fixed (datasheet 7.1.3)

bool SensorINA226::writeRegister16(uint8_t reg, uint16_t value)
{
  Wire.beginTransmission(INA226_I2C_ADDRESS);
  Wire.write(reg);
  Wire.write((uint8_t)(value >> 8));   // MSB dulu (datasheet 6.5.5.3)
  Wire.write((uint8_t)(value & 0xFF));
  return Wire.endTransmission() == 0;
}

bool SensorINA226::readRegister16(uint8_t reg, uint16_t &value)
{
  Wire.beginTransmission(INA226_I2C_ADDRESS);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) // repeated start
  {
    return false;
  }
  if (Wire.requestFrom((int)INA226_I2C_ADDRESS, 2) != 2)
  {
    return false;
  }
  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();
  value = ((uint16_t)msb << 8) | lsb;
  return true;
}

void SensorINA226::begin()
{
  Wire.begin(I2C_SDA, I2C_SCL); // sama bus dgn TF-Luna & AHT20, aman dipanggil ulang

  uint16_t id = 0;
  if (readRegister16(REG_MANUFACTURER_ID, id) && id == MANUFACTURER_ID_TI)
  {
    _healthy = true;
    _failCount = 0;
    _state = State::IDLE;
    logInfo("INA226", "Sensor terdeteksi di alamat I2C 0x%02X.", INA226_I2C_ADDRESS);
  }
  else
  {
    _healthy = false;
    logError("INA226", "Sensor TIDAK terdeteksi (alamat 0x%02X). Cek wiring I2C / pin A0-A1.", INA226_I2C_ADDRESS);
  }
  _lastRetryAt = millis();
}

void SensorINA226::update()
{
  unsigned long now = millis();

  if (!_healthy)
  {
    // Jangan block: coba re-init berkala saja (sama pola dgn AHT20/Lidar)
    if (now - _lastRetryAt >= INA226_RETRY_INTERVAL_MS)
    {
      logWarn("INA226", "Mencoba re-inisialisasi sensor...");
      begin();
    }
    return;
  }

  switch (_state)
  {
    case State::IDLE:
      if (now - _lastReadAt < INA226_READ_INTERVAL_MS)
      {
        return; // belum waktunya baca lagi
      }
      // Picu SATU konversi bus voltage (triggered mode) - langsung return,
      // TIDAK menunggu hasilnya di sini (itu yang bikin non-blocking).
      if (writeRegister16(REG_CONFIG, CONFIG_TRIGGER_BUS_VOLTAGE))
      {
        _triggeredAt = now;
        _state = State::WAITING_CONVERSION;
      }
      else
      {
        markFailure("Gagal memicu konversi (I2C write error)");
        _lastReadAt = now;
      }
      break;

    case State::WAITING_CONVERSION:
      if (now - _triggeredAt < INA226_CONVERSION_WAIT_MS)
      {
        return; // konversi di sensor belum tentu selesai, coba lagi loop() berikutnya
      }
      uint16_t raw = 0;
      if (readRegister16(REG_BUS_VOLTAGE, raw))
      {
        _busVoltageV = raw * BUS_VOLTAGE_LSB_V; // D15 selalu 0, bus voltage selalu positif
        markSuccess();
      }
      else
      {
        markFailure("Gagal membaca Bus Voltage Register");
      }
      _lastReadAt = now;
      _state = State::IDLE;
      break;
  }
}

void SensorINA226::markFailure(const char* reason)
{
  _failCount++;
  logWarn("INA226", "Gagal (%d/%d): %s", _failCount, INA226_MAX_FAIL_COUNT, reason);

  if (_failCount >= INA226_MAX_FAIL_COUNT)
  {
    _healthy = false;
    _lastRetryAt = millis();
    logError("INA226", "Sensor dianggap UNHEALTHY setelah %d kegagalan beruntun.", _failCount);
  }
}

void SensorINA226::markSuccess()
{
  _failCount = 0;
}

bool SensorINA226::isHealthy()      { return _healthy; }
float SensorINA226::getBusVoltageV(){ return _busVoltageV; }