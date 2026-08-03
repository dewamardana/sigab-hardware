/*
  ============================================================
  EspCamTrigger.h - Trigger ESP32-CAM lewat UART2 (hardware Serial)

  ESP32-S3 me-routing SEMUA UART (termasuk UART2) lewat GPIO Matrix -
  artinya UART2 BISA dipasang ke GPIO mana pun yang masih bebas, tidak
  terikat ke pin "default" tertentu di datasheet. Datasheet cuma
  menunjukkan pin default KALAU kita tidak menentukan pin sendiri saat
  .begin(). Project ini sudah pakai UART1 utk GPS (lihat SensorGPS.h,
  HardwareSerial _serial{1}) - di sini kita pakai UART2 yang masih
  sepenuhnya bebas, di-route ke PIN_CAM_RX/PIN_CAM_TX (Config.h).
  TIDAK PERLU library tambahan - HardwareSerial sudah bawaan ESP32 core.

  ALUR: requestCapture() TIDAK langsung kirim perintah - ia cuma
  menandai "ada permintaan", lalu update() (dipanggil tiap loop())
  yang benar-benar mengirim perintah SETELAH CAM_FLASH_WARMUP_MS
  berlalu (non-blocking, pakai millis(), BUKAN delay()). Menyalakan
  flash itu sendiri dilakukan di RiverMonitor.ino SEBELUM memanggil
  requestCapture() - kelas ini cuma urus pengiriman perintahnya saja.

  CATATAN: ini baru mengirim PERINTAH ke ESP32-CAM. ESP32-CAM perlu
  firmware terpisah yang mendengarkan UART-nya & mengenali
  CAM_CAPTURE_CMD untuk benar-benar mengambil gambar - itu di luar
  cakupan kode ini (kabari saya kalau butuh dibuatkan).
  ============================================================
*/
#ifndef ESP_CAM_TRIGGER_H
#define ESP_CAM_TRIGGER_H

#include <Arduino.h>
#include "Config.h"

class EspCamTrigger {
  public:
    void begin();
    void update();          // non-blocking, panggil tiap loop()
    void requestCapture();  // panggil SEKALI tiap mau ambil gambar (mis. saat alarm baru aktif)

  private:
    HardwareSerial _camSerial{2}; // UART2 - masih bebas, tidak bentrok dgn GPS (UART1) atau debug (UART0)
    bool _pending = false;
    unsigned long _requestedAt = 0;
};

#endif // ESP_CAM_TRIGGER_H