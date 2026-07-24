#include "SensorLidar.h"
#include "Logger.h"

// Command: minta 1x pembacaan (0x5A 0x05 0x00 0x01 checksum)
static unsigned char CMD_SINGLE_READ[] = { 0x5A, 0x05, 0x00, 0x01, 0x60 };

void SensorLidar::begin()
{
  Wire.begin(I2C_SDA, I2C_SCL);
  // Cek apakah alamat I2C merespon (deteksi wiring salah lebih awal)
  Wire.beginTransmission(TFLUNA_I2C_ADDRESS);
  uint8_t err = Wire.endTransmission();

  if (err == 0)
  {
    _healthy = true;
    logInfo("TFLUNA", "Sensor terdeteksi di alamat I2C 0x%02X", TFLUNA_I2C_ADDRESS);
  }
  else
  {
    _healthy = false;
    logError("TFLUNA", "Sensor TIDAK terdeteksi (I2C error=%d). Cek wiring SDA/SCL.", err);
  }
  _lastRetryAt = millis();
}

bool SensorLidar::readOnce()
{
  Wire.beginTransmission(TFLUNA_I2C_ADDRESS);
  Wire.write(CMD_SINGLE_READ, 5);
  if (Wire.endTransmission() != 0)
  {
    return false; // gagal kirim perintah (sensor tidak menjawab)
  }

  Wire.requestFrom(TFLUNA_I2C_ADDRESS, TFLUNA_DATA_LENGTH);

  uint8_t data[TFLUNA_DATA_LENGTH] = { 0 };
  int index = 0;
  while (Wire.available() > 0 && index < TFLUNA_DATA_LENGTH)
  {
    data[index++] = Wire.read();
  }

  if (index != TFLUNA_DATA_LENGTH)
  {
    return false; // data tidak lengkap
  }

  _distanceCM   = data[2] + data[3] * 256;
  _strength     = data[4] + data[5] * 256;
  int16_t rawT  = data[6] + data[7] * 256;
  _temperatureC = rawT / 8.0f - 256.0f;

  // Signal strength terlalu rendah biasanya berarti target di luar jangkauan/terhalang
  if (_strength < 100)
  {
    return false;
  }

  return true;
}

void SensorLidar::update()
{
  unsigned long now = millis();

  if (!_healthy)
  {
    // Jangan block: coba re-init berkala saja
    if (now - _lastRetryAt >= TFLUNA_RETRY_INTERVAL_MS)
    {
      logWarn("TFLUNA", "Mencoba re-inisialisasi sensor...");
      begin();
    }
    return;
  }

  if (now - _lastReadAt < TFLUNA_READ_INTERVAL_MS)
  {
    return; // belum waktunya baca lagi
  }
  _lastReadAt = now;

  if (readOnce())
  {
    markSuccess();
  }
  else
  {
    markFailure("Timeout/checksum/data tidak lengkap");
  }
}

void SensorLidar::markFailure(const char* reason)
{
  _failCount++;
  logWarn("TFLUNA", "Gagal baca (%d/%d): %s", _failCount, TFLUNA_MAX_FAIL_COUNT, reason);

  if (_failCount >= TFLUNA_MAX_FAIL_COUNT)
  {
    _healthy = false;
    _lastRetryAt = millis();
    logError("TFLUNA", "Sensor dianggap UNHEALTHY setelah %d kegagalan beruntun.", _failCount);
  }
}

void SensorLidar::markSuccess()
{
  _failCount = 0;
}

bool SensorLidar::isHealthy()      { return _healthy; }
uint16_t SensorLidar::getDistanceCM()   { return _distanceCM; }
uint16_t SensorLidar::getStrength()     { return _strength; }
float SensorLidar::getTemperatureC()    { return _temperatureC; }
