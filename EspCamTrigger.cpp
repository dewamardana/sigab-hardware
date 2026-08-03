#include "EspCamTrigger.h"
#include "Logger.h"

void EspCamTrigger::begin()
{
  // Custom pin assignment - ini yang membuat UART2 "pindah" dari pin
  // default datasheet ke PIN_CAM_RX/PIN_CAM_TX pilihan kita sendiri.
  _camSerial.begin(CAM_BAUD, SERIAL_8N1, PIN_CAM_RX, PIN_CAM_TX);
  logInfo("CAM", "UART2 ke ESP32-CAM siap (RX=%d, TX=%d, baud=%d)",
          PIN_CAM_RX, PIN_CAM_TX, CAM_BAUD);
}

void EspCamTrigger::requestCapture()
{
  if (_pending)
  {
    return;
  }
  _pending = true;
  _requestedAt = millis();
  logInfo("CAM", "Permintaan capture diterima, menunggu warmup flash %lu ms...", CAM_FLASH_WARMUP_MS);
}

void EspCamTrigger::update()
{
  if (!_pending)
  {
    return;
  }

  if (millis() - _requestedAt >= CAM_FLASH_WARMUP_MS)
  {
    _camSerial.print(CAM_CAPTURE_CMD);
    logInfo("CAM", "Perintah ambil gambar terkirim ke ESP32-CAM.");
    _pending = false;
  }
}