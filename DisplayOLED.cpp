#include "DisplayOLED.h"
#include "Logger.h"

// ------------------------------------------------------------
// begin()
// Inisialisasi layar. Kalau OLED_MODE == 0, sengaja dilewati
// total (tidak sentuh I2C sama sekali) supaya kalau OLED belum
// terpasang secara fisik, sistem tetap jalan normal.
// ------------------------------------------------------------
void DisplayOLED::begin()
{
#if OLED_MODE == 0
  logInfo("OLED", "OLED_MODE=0 -> layar tidak dipakai, lewati inisialisasi.");
  _healthy = false;
#else
  // Wire.begin aman dipanggil berulang - dipakai bareng TF-Luna/AHT20 di bus I2C yang sama.
  Wire.begin(I2C_SDA, I2C_SCL);

  if (_oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS))
  {
    _healthy = true;
    _oled.clearDisplay();
    _oled.setTextColor(SSD1306_WHITE);
    _oled.setTextSize(1);
    _oled.setCursor(0, 24);
    _oled.println("River Monitor");
    _oled.println("Booting...");
    _oled.display();
    logInfo("OLED", "Layar terdeteksi di alamat I2C 0x%02X (OLED_MODE=%d)", OLED_I2C_ADDRESS, OLED_MODE);
  }
  else
  {
    _healthy = false;
    logError("OLED", "Layar TIDAK terdeteksi di alamat 0x%02X. Cek wiring SDA/SCL & alamat I2C.", OLED_I2C_ADDRESS);
  }
  _lastRetryAt = millis();
#endif
}

// ------------------------------------------------------------
// update()
// Dipanggil tiap iterasi loop() - non-blocking:
//   - kalau belum sehat, coba re-init berkala saja (tidak block)
//   - kalau sehat, gambar ulang layar tiap OLED_UPDATE_INTERVAL_MS
// Konten yang digambar tergantung OLED_MODE (dipilih saat compile).
// ------------------------------------------------------------
void DisplayOLED::update(const RiverData& data, bool wifiConnected, bool mqttConnected)
{
#if OLED_MODE == 0
  return;
#else
  unsigned long now = millis();

  if (!_healthy)
  {
    if (now - _lastRetryAt >= OLED_RETRY_INTERVAL_MS)
    {
      logWarn("OLED", "Mencoba re-inisialisasi layar...");
      begin();
    }
    return;
  }

  if (now - _lastUpdateAt < OLED_UPDATE_INTERVAL_MS)
  {
    return; // belum waktunya refresh lagi
  }
  _lastUpdateAt = now;

  _oled.clearDisplay();

  #if OLED_MODE == 1
    // Mode DEBUG: gilir halaman karena informasinya banyak
    if (now - _lastPageChangeAt >= OLED_DEBUG_PAGE_INTERVAL_MS)
    {
      _lastPageChangeAt = now;
      _debugPage = (_debugPage + 1) % DEBUG_PAGE_COUNT;
    }
    drawDebugPage(data, wifiConnected, mqttConnected);
  #elif OLED_MODE == 2
    drawNormalPage(data, wifiConnected, mqttConnected);
  #endif

  _oled.display();
#endif
}

bool DisplayOLED::isHealthy() { return _healthy; }

// ------------------------------------------------------------
// Header kecil di baris paling atas: judul halaman + status
// WiFi/MQTT singkat ("W"/"M" nyala = terhubung). Dipakai di
// kedua mode supaya konsisten.
// ------------------------------------------------------------
void DisplayOLED::drawHeader(const char* judul, bool wifiConnected, bool mqttConnected)
{
  _oled.setTextSize(1);
  _oled.setCursor(0, 0);
  _oled.print(judul);

  _oled.setCursor(OLED_WIDTH - 18, 0);
  _oled.print(wifiConnected ? "W" : "-");
  _oled.print(mqttConnected ? "M" : "-");

  _oled.drawLine(0, 10, OLED_WIDTH - 1, 10, SSD1306_WHITE);
}

// ------------------------------------------------------------
// Mode DEBUG - 3 halaman bergantian berisi data MENTAH, buat
// troubleshooting/commissioning di lapangan:
//   Halaman 1: Lingkungan    (suhu, kelembapan, angin, hujan)
//   Halaman 2: Air & Baterai (TMA, level kritis, tegangan baterai)
//   Halaman 3: Konektivitas  (WiFi, MQTT, GPS fix + koordinat)
// ------------------------------------------------------------
void DisplayOLED::drawDebugPage(const RiverData& data, bool wifiConnected, bool mqttConnected)
{
  _oled.setTextSize(1);

  if (_debugPage == 0)
  {
    drawHeader("D1 Lingkungan", wifiConnected, mqttConnected);
    _oled.setCursor(0, 16);
    _oled.printf("Suhu   : %.1f C\n", data.suhu);
    _oled.printf("Lembap : %.1f %%RH\n", data.kelembapan);
    _oled.printf("Angin  : %.1f km/j\n", data.angin_kmph);
    _oled.printf("Hujan  : %.1f mm/j\n", data.hujan_intensitas_mmh);
  }
  else if (_debugPage == 1)
  {
    drawHeader("D2 Air&Baterai", wifiConnected, mqttConnected);
    _oled.setCursor(0, 16);
    _oled.printf("TMA(cm)  : %.1f\n", data.tma_cm);
    _oled.printf("Freeboard: %.2f m\n", data.freeboard_m);
    _oled.printf("Status   : %s\n", data.statusLabel.c_str());
    _oled.printf("Kritis   : %s\n", data.levelKritis ? "YA!" : "tidak");
    _oled.printf("Baterai  : %.2f V\n", data.baterai_v);
  }
  else
  {
    drawHeader("D3 Koneksi", wifiConnected, mqttConnected);
    _oled.setCursor(0, 16);
    _oled.printf("WiFi : %s\n", wifiConnected ? "terhubung" : "terputus");
    _oled.printf("MQTT : %s\n", mqttConnected ? "terhubung" : "terputus");
    if (data.gpsFix)
    {
      _oled.printf("GPS  : %.4f,\n       %.4f\n", data.gpsLat, data.gpsLng);
    }
    else
    {
      _oled.println("GPS  : belum fix");
    }
  }
}

// ------------------------------------------------------------
// Mode NORMAL - satu layar ringkas untuk pemantauan harian oleh
// operator non-teknis: TMA ditampilkan besar (data paling
// krusial), lalu baris ringkas level kritis, baterai, dan hujan.
// ------------------------------------------------------------
void DisplayOLED::drawNormalPage(const RiverData& data, bool wifiConnected, bool mqttConnected)
{
  drawHeader("River Monitor", wifiConnected, mqttConnected);

  // Status fuzzy (NORMAL/SIAGA/BAHAYA) ditampilkan BESAR - ini "data paling
  // krusial" yang sebenarnya (sudah mensintesis TMA+hujan), lebih bermakna
  // bagi operator non-teknis dibanding angka freeboard mentah.
  _oled.setTextSize(2);
  _oled.setCursor(0, 14);
  _oled.printf("%s", data.statusLabel.c_str());

  _oled.setTextSize(1);
  _oled.setCursor(0, 34);
  _oled.printf("Freeboard: %.2f m\n", data.freeboard_m);
  _oled.println(data.levelKritis ? "!! LEVEL KRITIS !!" : "Level: normal");
  _oled.printf("Baterai: %.2f V\n", data.baterai_v);
}
