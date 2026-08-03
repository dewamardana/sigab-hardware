/*
  ============================================================
  DisplayOLED.h - Tampilan status ke layar OLED I2C (SSD1306)
  ============================================================
  OLED berbagi bus I2C yang sama dengan TF-Luna & AHT20 (lihat
  I2C_SDA/I2C_SCL di Config.h) - alamat I2C beda jadi aman satu bus.

  Mode tampilan diatur lewat OLED_MODE di Config.h:
    0 = OFF    -> OLED tidak diinisialisasi & tidak dipakai sama sekali
    1 = DEBUG  -> tampilkan data MENTAH semua sensor + status koneksi,
                  digilir 3 halaman (banyak info, cocok dipakai
                  teknisi saat instalasi/troubleshooting di lapangan)
    2 = NORMAL -> tampilkan ringkasan penting saja dalam satu layar
                  (TMA, baterai, kategori hujan, WiFi/MQTT) - cocok
                  untuk pemantauan harian oleh operator non-teknis

  Cara pakai (lihat juga contoh lengkap di RiverMonitor.ino):

    #include "DisplayOLED.h"
    DisplayOLED displayOLED;

    void setup() {
      displayOLED.begin();
    }

    void loop() {
      // panggil tiap iterasi - internal sudah non-blocking &
      // throttled sendiri lewat OLED_UPDATE_INTERVAL_MS
      displayOLED.update(data, wifiManager.isConnected(), mqttManager.isConnected());
    }

  PENTING - Library yang wajib di-install lewat Arduino Library Manager:
    - "Adafruit SSD1306"
    - "Adafruit GFX Library"

  Kalau modul OLED yang dipakai memakai chip SH1106 (bukan SSD1306 -
  cek dus/label pembelian, keduanya mirip fisiknya tapi TIDAK
  kompatibel), ganti library ke "Adafruit SH110X" lalu ganti tipe
  _oled di bawah menjadi Adafruit_SH1106G dan begin(...) sesuaikan
  argumennya - selebihnya (drawDebugPage/drawNormalPage) tetap sama
  karena API Adafruit_GFX-nya identik.
  ============================================================
*/
#ifndef DISPLAY_OLED_H
#define DISPLAY_OLED_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Config.h"
#include "RiverData.h"

class DisplayOLED {
  public:
    void begin();

    // Non-blocking: hanya benar-benar menggambar ulang layar tiap
    // OLED_UPDATE_INTERVAL_MS. Aman & murah dipanggil tiap iterasi loop().
    void update(const RiverData& data, bool wifiConnected, bool mqttConnected);

    bool isHealthy();

  private:
    Adafruit_SSD1306 _oled = Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET_PIN);

    bool _healthy = false;
    unsigned long _lastUpdateAt = 0;
    unsigned long _lastRetryAt = 0;
    unsigned long _lastPageChangeAt = 0;
    uint8_t _debugPage = 0;
    static const uint8_t DEBUG_PAGE_COUNT = 3;

    void drawHeader(const char* judul, bool wifiConnected, bool mqttConnected);
    void drawDebugPage(const RiverData& data, bool wifiConnected, bool mqttConnected);
    void drawNormalPage(const RiverData& data, bool wifiConnected, bool mqttConnected);
};

#endif // DISPLAY_OLED_H
