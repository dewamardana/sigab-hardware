/*
  ============================================================
  SensorTest.h - Mode pengujian sensor secara terpisah
  ============================================================
  Tujuan file ini:
    1. testSensor(nama)   -> uji SATU sensor tertentu saja, hasil
                             dicetak berulang ke Serial Monitor.
                             Berguna saat commissioning/wiring baru
                             tanpa perlu menyalakan WiFi/MQTT.
    2. testAllSensors()   -> baca SEMUA sensor lalu tampilkan
                             payload JSON yang AKAN dikirim ke MQTT,
                             tapi TIDAK benar-benar mengirim apa pun
                             dan TIDAK memerlukan WiFi/MQTT aktif.

  Cara pakai (lihat juga contoh di RiverMonitor.ino):

    #include "SensorTest.h"

    void setup() {
      Serial.begin(115200);
      delay(300);

      // --- Mode A: uji satu sensor saja ---
      // sensorLidar.begin();
      // while (true) { testSensor("lidar"); delay(200); }

      // --- Mode B: uji semua sensor, tampilkan payload JSON ---
      // sensorRain.begin(); sensorLidar.begin(); sensorAHT20.begin();
      // sensorWind.begin(); sensorFloat.begin(); sensorGPS.begin();
      // sensorBaterai.begin();
      // testAllSensors();   // panggil sekali, atau taruh di loop() dgn interval
    }

  PENTING:
    - File ini TIDAK membuat objek sensor sendiri. Ia memakai objek
      global (sensorRain, sensorLidar, dst) yang sudah dideklarasikan
      di RiverMonitor.ino, supaya tidak ada dua instance yang rebutan
      pin/I2C/interrupt yang sama.
    - testSensor() memanggil begin() otomatis sekali di awal jika
      sensor belum pernah di-begin (aman dipanggil berulang, tidak
      re-init tiap kali).
    - Nama sensor bersifat case-insensitive: "lidar", "LIDAR", "Lidar"
      semua diterima.
  ============================================================
*/
#ifndef SENSOR_TEST_H
#define SENSOR_TEST_H

#include <Arduino.h>
#include "Config.h"
#include "Logger.h"
#include "RiverData.h"
#include "JsonBuilder.h"
#include "SensorRain.h"
#include "SensorLidar.h"
#include "SensorAHT20.h"
#include "SensorWind.h"
#include "SensorFloatSwitch.h"
#include "SensorGPS.h"
#include "SensorAnalog.h"

// ------------------------------------------------------------
// Objek sensor dipakai dari luar (dideklarasikan di RiverMonitor.ino).
// "extern" berarti: pakai objek yang SUDAH ada, jangan buat baru.
// ------------------------------------------------------------
extern SensorRain        sensorRain;
extern SensorLidar       sensorLidar;
extern SensorAHT20       sensorAHT20;
extern SensorWind        sensorWind;
extern SensorFloatSwitch sensorFloat;
extern SensorGPS         sensorGPS;
extern SensorAnalog      sensorBaterai;

// ------------------------------------------------------------
// Helper internal: pastikan sensor yang mau diuji sudah di-begin().
// Supaya testSensor() bisa dipanggil langsung dari setup() tanpa
// perlu begin() manual dulu untuk sensor yang bersangkutan.
// ------------------------------------------------------------
namespace SensorTestInternal {
  inline bool& flagFor(const String& s)
  {
    static bool doneRain = false, doneLidar = false, doneAHT = false,
                doneWind = false, doneFloat = false, doneGPS = false,
                doneBat = false;
    if (s == "rain")   return doneRain;
    if (s == "lidar")  return doneLidar;
    if (s == "aht20")  return doneAHT;
    if (s == "wind")   return doneWind;
    if (s == "float")  return doneFloat;
    if (s == "gps")    return doneGPS;
    if (s == "bat" || s == "baterai" || s == "battery") return doneBat;
    static bool dummy = false;
    return dummy;
  }
}

// ------------------------------------------------------------
// testSensor(nama)
// Uji SATU sensor tertentu, cetak hasil pembacaannya ke Serial.
// Panggil fungsi ini berulang (mis. di dalam loop() atau while(true)
// + delay kecil) supaya datanya ter-update terus, mirip live monitor.
//
// nama yang didukung:
//   "rain"    -> Rain gauge (tipping bucket)
//   "lidar"   -> TF-Luna (ketinggian air)
//   "aht20"   -> Suhu & kelembapan
//   "wind"    -> Anemometer
//   "float"   -> Float switch (level kritis)
//   "gps"     -> GPS (lokasi)
//   "bat"     -> Tegangan baterai
// ------------------------------------------------------------
inline void testSensor(String nama)
{
  nama.toLowerCase();
  bool &sudahBegin = SensorTestInternal::flagFor(nama);

  if (nama == "rain")
  {
    if (!sudahBegin) { sensorRain.begin(); sudahBegin = true; }
    sensorRain.update();
    Serial.println("---- [TEST] Rain Gauge ----");
    Serial.printf("  Healthy       : %s\n", sensorRain.isHealthy() ? "YA" : "TIDAK");
    Serial.printf("  Intensitas    : %.2f mm/jam\n", sensorRain.getIntensityMMh());
    Serial.printf("  Kategori      : %s\n", sensorRain.getCategory().c_str());
    Serial.printf("  Total (mm)    : %.2f\n", sensorRain.getTotalMM());
    Serial.printf("  Tip terakhir  : %lu\n", sensorRain.getLastTipCount());
  }
  else if (nama == "lidar")
  {
    if (!sudahBegin) { sensorLidar.begin(); sudahBegin = true; }
    sensorLidar.update();
    Serial.println("---- [TEST] TF-Luna LiDAR ----");
    Serial.printf("  Healthy       : %s\n", sensorLidar.isHealthy() ? "YA" : "TIDAK");
    Serial.printf("  Jarak         : %u cm\n", sensorLidar.getDistanceCM());
    Serial.printf("  Kekuatan sinyal: %u\n", sensorLidar.getStrength());
    Serial.printf("  Suhu internal : %.2f C\n", sensorLidar.getTemperatureC());
    float tma = TFLUNA_TINGGI_PEMASANGAN_CM - sensorLidar.getDistanceCM();
    Serial.printf("  Estimasi TMA  : %.1f cm (tinggi pemasangan %.1f cm)\n",
                  tma, TFLUNA_TINGGI_PEMASANGAN_CM);
  }
  else if (nama == "aht20")
  {
    if (!sudahBegin) { sensorAHT20.begin(); sudahBegin = true; }
    sensorAHT20.update();
    Serial.println("---- [TEST] AHT20 (Suhu & Kelembapan) ----");
    Serial.printf("  Healthy       : %s\n", sensorAHT20.isHealthy() ? "YA" : "TIDAK");
    Serial.printf("  Suhu          : %.2f C\n", sensorAHT20.getTemperatureC());
    Serial.printf("  Kelembapan    : %.2f %%RH\n", sensorAHT20.getHumidityRH());
  }
  else if (nama == "wind")
  {
    if (!sudahBegin) { sensorWind.begin(); sudahBegin = true; }
    sensorWind.update();
    Serial.println("---- [TEST] Anemometer (Kecepatan Angin) ----");
    Serial.printf("  Healthy       : %s\n", sensorWind.isHealthy() ? "YA" : "TIDAK");
    Serial.printf("  Kecepatan     : %.2f m/s (%.2f km/jam)\n",
                  sensorWind.getSpeedMS(), sensorWind.getSpeedKMH());
    Serial.printf("  RPS           : %.2f\n", sensorWind.getRPS());
  }
  else if (nama == "float")
  {
    if (!sudahBegin) { sensorFloat.begin(); sudahBegin = true; }
    sensorFloat.update();
    Serial.println("---- [TEST] Float Switch (Level Kritis) ----");
    Serial.printf("  Healthy       : %s\n", sensorFloat.isHealthy() ? "YA" : "TIDAK");
    Serial.printf("  Status        : %s\n", sensorFloat.isWaterHigh() ? "AIR TINGGI (kritis)" : "normal");
  }
  else if (nama == "gps")
  {
    if (!sudahBegin) { sensorGPS.begin(); sudahBegin = true; }
    sensorGPS.update();
    Serial.println("---- [TEST] GPS ----");
    Serial.printf("  Healthy (ada data masuk): %s\n", sensorGPS.isHealthy() ? "YA" : "TIDAK");
    Serial.printf("  Fix           : %s\n", sensorGPS.hasFix() ? "YA" : "BELUM");
    Serial.printf("  Satelit       : %lu\n", (unsigned long)sensorGPS.getSatellites());
    if (sensorGPS.hasFix())
    {
      Serial.printf("  Lat/Lng       : %.6f, %.6f\n", sensorGPS.getLat(), sensorGPS.getLng());
    }
  }
  else if (nama == "bat" || nama == "baterai" || nama == "battery")
  {
    if (!sudahBegin) { sensorBaterai.begin(); sudahBegin = true; }
    Serial.println("---- [TEST] Tegangan Baterai ----");
    Serial.printf("  Healthy       : %s\n", sensorBaterai.isHealthy() ? "YA" : "TIDAK");
    Serial.printf("  Tegangan      : %.2f V\n", sensorBaterai.read());
  }
  else
  {
    Serial.println("---- [TEST] Nama sensor tidak dikenali ----");
    Serial.println("  Pilihan valid: rain, lidar, aht20, wind, float, gps, bat");
  }

  Serial.println();
}

// ------------------------------------------------------------
// testAllSensors()
// Baca SEMUA sensor lalu tampilkan ke Serial:
//   1. Ringkasan status kesehatan tiap sensor (sama seperti
//      printSystemStatus() di RiverMonitor.ino), dan
//   2. Payload JSON PERSIS seperti yang akan dikirim ke MQTT
//      (dibangun lewat buatJSON() yang sama dengan kode produksi),
//      TAPI TIDAK PERNAH dipublish/dikirim kemana pun.
//
// Fungsi ini TIDAK memanggil WiFiManager atau MqttManager sama
// sekali, jadi tidak butuh WiFi/broker MQTT untuk dijalankan -
// cocok dipakai di lokasi tanpa jaringan saat commissioning.
//
// Catatan: panggil begin() untuk semua sensor dulu di setup()
// sebelum memanggil ini (lihat contoh di komentar atas file).
// ------------------------------------------------------------
inline void testAllSensors()
{
  // Update semua sensor sekali (non-blocking, sama seperti loop() asli)
  sensorRain.update();
  sensorLidar.update();
  sensorAHT20.update();
  sensorWind.update();
  sensorFloat.update();
  sensorGPS.update();

  // Kumpulkan snapshot data, PERSIS logika ambilDataSensor() di .ino,
  // supaya payload yang ditampilkan representatif dengan kondisi asli.
  RiverData data;

  if (sensorAHT20.isHealthy())
  {
    data.suhu       = sensorAHT20.getTemperatureC();
    data.kelembapan = sensorAHT20.getHumidityRH();
  }

  if (sensorLidar.isHealthy())
  {
    data.tma_cm = TFLUNA_TINGGI_PEMASANGAN_CM - sensorLidar.getDistanceCM();
  }

  data.angin_kmph           = sensorWind.getSpeedKMH();
  data.hujan_mm             = sensorRain.getTotalMM();
  data.hujan_intensitas_mmh = sensorRain.getIntensityMMh();
  data.hujan_kategori       = sensorRain.getCategory();
  data.levelKritis          = sensorFloat.isWaterHigh();
  data.baterai_v            = sensorBaterai.read();

  data.gpsFix = sensorGPS.hasFix();
  if (data.gpsFix)
  {
    data.gpsLat = sensorGPS.getLat();
    data.gpsLng = sensorGPS.getLng();
  }

  // --- 1. Ringkasan status kesehatan tiap sensor ---
  Serial.println("========== [TEST ALL SENSORS] Ringkasan Status ==========");
  Serial.printf("Rain Gauge  : %s | %.2f mm/jam | %s | total=%.1f mm\n",
                sensorRain.isHealthy() ? "OK" : "ERROR",
                sensorRain.getIntensityMMh(), sensorRain.getCategory().c_str(),
                sensorRain.getTotalMM());
  Serial.printf("TF-Luna     : %s | jarak=%u cm, tma=%.1f cm, sinyal=%u\n",
                sensorLidar.isHealthy() ? "OK" : "ERROR",
                sensorLidar.getDistanceCM(), data.tma_cm, sensorLidar.getStrength());
  Serial.printf("AHT20       : %s | %.2f C, %.2f %%RH\n",
                sensorAHT20.isHealthy() ? "OK" : "ERROR",
                sensorAHT20.getTemperatureC(), sensorAHT20.getHumidityRH());
  Serial.printf("Anemometer  : %s | %.2f m/s (%.2f km/jam)\n",
                sensorWind.isHealthy() ? "OK" : "ERROR",
                sensorWind.getSpeedMS(), sensorWind.getSpeedKMH());
  Serial.printf("Float Switch: %s | %s\n",
                sensorFloat.isHealthy() ? "OK" : "ERROR",
                sensorFloat.isWaterHigh() ? "AIR TINGGI" : "normal");
  Serial.printf("GPS         : %s | fix=%s, satelit=%lu\n",
                sensorGPS.isHealthy() ? "OK" : "ERROR (tidak ada data masuk)",
                sensorGPS.hasFix() ? "ya" : "belum",
                (unsigned long)sensorGPS.getSatellites());
  Serial.printf("Baterai     : %.2f V\n", data.baterai_v);

  // --- 2. Payload JSON yang AKAN dikirim ke MQTT (tapi tidak dikirim) ---
  String payload = buatJSON(data);
  Serial.println("---------- Payload JSON (simulasi, TIDAK dikirim) ----------");
  Serial.printf("Topic tujuan (jika dikirim): %s\n", TOPIC_SENSOR);
  Serial.println(payload);
  Serial.println("=============================================================");
  Serial.println();
}

#endif // SENSOR_TEST_H
