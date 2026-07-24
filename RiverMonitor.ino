/*
  ============================================================
  RiverMonitor.ino
  Sistem Monitoring Sungai - ESP32-S3
  ============================================================
  Struktur:
    Config.h            -> semua pin, kredensial, & parameter timing
    Logger.h            -> debug print terstruktur (ERROR/WARN/INFO/DEBUG)
    WiFiManager.*        -> koneksi WiFi non-blocking + auto reconnect
    MqttManager.*        -> koneksi & publish MQTT non-blocking + auto reconnect
    RiverData.h          -> struct snapshot semua nilai sensor terkini
    JsonBuilder.*        -> membangun payload JSON dari RiverData
    SensorRain.*         -> rain gauge tipping bucket (kategori + akumulasi total)
    SensorLidar.*        -> TF-Luna I2C (jarak -> tinggi muka air)
    SensorAHT20.*        -> suhu & kelembapan
    SensorWind.*         -> anemometer (kecepatan angin)
    SensorFloatSwitch.*  -> saklar pelampung (level air kritis)
    SensorGPS.*          -> lokasi titik pemantauan
    SensorAnalog.*       -> pembacaan tegangan baterai (voltage divider)

  Prinsip ketersediaan tinggi (high availability) yang dipakai:
   1. TIDAK ADA blocking while(true)/delay(lama) di loop utama. Kalau
      sensor atau koneksi gagal, ditandai UNHEALTHY/DISCONNECTED dan
      modul lain tetap jalan.
   2. Setiap modul sensor mencoba re-inisialisasi sendiri secara
      berkala (non-blocking) sampai sensor kembali terbaca.
   3. WiFi & MQTT masing-masing punya state machine sendiri yang
      mencoba reconnect di background; sensor tetap dibaca selama itu.
   4. Data yang dikirim ke MQTT diambil LANGSUNG dari pembacaan sensor
      asli (bukan simulasi/manual) - snapshot dikumpulkan ke RiverData
      tiap siklus kirim.
   5. Status kesehatan tiap sensor & koneksi dicetak berkala lewat
      Logger, supaya mudah dipantau saat commissioning di lapangan.
  ============================================================
*/

#include "Config.h"
#include "Logger.h"
#include "WiFiManager.h"
#include "MqttManager.h"
#include "RiverData.h"
#include "JsonBuilder.h"
#include "SensorRain.h"
#include "SensorLidar.h"
#include "SensorAHT20.h"
#include "SensorWind.h"
#include "SensorFloatSwitch.h"
#include "SensorGPS.h"
#include "SensorAnalog.h"
#include "SensorTest.h"

WiFiManager wifiManager;
MqttManager mqttManager;
SensorRain sensorRain;
SensorLidar sensorLidar;
SensorAHT20 sensorAHT20;
SensorWind sensorWind;
SensorFloatSwitch sensorFloat;
SensorGPS sensorGPS;
SensorAnalog sensorBaterai(PIN_POT_BATERAI, BATERAI_RENTANG_V, BATERAI_MIN_V);

RiverData data;

unsigned long lastStatusPrint = 0;
unsigned long lastKirimMqtt = 0;

// ------------------------------------------------------------
// Ambil data langsung dari semua sensor asli ke dalam satu snapshot.
// Kalau sebuah sensor sedang UNHEALTHY, nilai lamanya sengaja
// dibiarkan (tidak ditimpa dengan 0), supaya payload tidak
// tiba-tiba menampilkan nilai kosong/salah saat sensor bermasalah.
// ------------------------------------------------------------
void ambilDataSensor() {
  if (sensorAHT20.isHealthy()) {
    data.suhu = sensorAHT20.getTemperatureC();
    data.kelembapan = sensorAHT20.getHumidityRH();
  }

  if (sensorLidar.isHealthy()) {
    data.tma_cm = TFLUNA_TINGGI_PEMASANGAN_CM - sensorLidar.getDistanceCM();
  }

  data.angin_kmph = sensorWind.getSpeedKMH();
  data.hujan_mm = sensorRain.getTotalMM();
  data.hujan_intensitas_mmh = sensorRain.getIntensityMMh();
  data.hujan_kategori = sensorRain.getCategory();
  data.levelKritis = sensorFloat.isWaterHigh();
  data.baterai_v = sensorBaterai.read();

  data.gpsFix = sensorGPS.hasFix();
  if (data.gpsFix) {
    data.gpsLat = sensorGPS.getLat();
    data.gpsLng = sensorGPS.getLng();
  }
}

// ------------------------------------------------------------
// Bangun payload dari snapshot terkini & kirim lewat MQTT.
// Aman dipanggil kapan saja - MqttManager sendiri yang menahan
// pengiriman kalau belum tersambung (lihat log [MQTT]).
// ------------------------------------------------------------
void kirimDataKeServer() {
  ambilDataSensor();
  String payload = buatJSON(data);
  mqttManager.publish(TOPIC_SENSOR, payload.c_str());
}

// ------------------------------------------------------------
// Cetak ringkasan status seluruh sistem (WiFi + MQTT + kesehatan sensor)
// ------------------------------------------------------------
void printSystemStatus() {
  logInfo("SYSTEM", "==================== STATUS SISTEM ====================");
  logInfo("SYSTEM", "WiFi        : %s", wifiManager.isConnected() ? "TERHUBUNG" : "TERPUTUS (reconnect berjalan)");
  logInfo("SYSTEM", "MQTT        : %s", mqttManager.isConnected() ? "TERHUBUNG" : "TERPUTUS (reconnect berjalan)");
  logInfo("SYSTEM", "Rain Gauge  : %s | %.2f mm/jam | %s | total=%.1f mm",
          sensorRain.isHealthy() ? "OK" : "ERROR",
          sensorRain.getIntensityMMh(), sensorRain.getCategory().c_str(), sensorRain.getTotalMM());
  logInfo("SYSTEM", "TF-Luna     : %s | jarak=%d cm, tma=%.1f cm, kekuatan sinyal=%d",
          sensorLidar.isHealthy() ? "OK" : "ERROR",
          sensorLidar.getDistanceCM(), data.tma_cm, sensorLidar.getStrength());
  logInfo("SYSTEM", "AHT20       : %s | %.2f C, %.2f %%RH",
          sensorAHT20.isHealthy() ? "OK" : "ERROR",
          sensorAHT20.getTemperatureC(), sensorAHT20.getHumidityRH());
  logInfo("SYSTEM", "Anemometer  : %s | %.2f m/s (%.2f km/jam)",
          sensorWind.isHealthy() ? "OK" : "ERROR",
          sensorWind.getSpeedMS(), sensorWind.getSpeedKMH());
  logInfo("SYSTEM", "Float Switch: %s | %s",
          sensorFloat.isHealthy() ? "OK" : "ERROR",
          sensorFloat.isWaterHigh() ? "AIR TINGGI" : "normal");
  logInfo("SYSTEM", "GPS         : %s | fix=%s, satelit=%lu",
          sensorGPS.isHealthy() ? "OK" : "ERROR (tidak ada data masuk)",
          sensorGPS.hasFix() ? "ya" : "belum", (unsigned long)sensorGPS.getSatellites());
  logInfo("SYSTEM", "Baterai     : %.2f V", data.baterai_v);
  logInfo("SYSTEM", "========================================================");
}

void setup() {
  Serial.begin(115200);
  delay(300);

  logInfo("SYSTEM", "=== River Monitoring System - Booting ===");

  #if TEST_MODE == 1
    logInfo("SYSTEM", "TEST_MODE=1 -> menguji sensor '%s' saja, WiFi/MQTT tidak dipakai", TEST_SENSOR_NAME);
    while (true) {
      testSensor(TEST_SENSOR_NAME);
      delay(TEST_MODE_INTERVAL_MS);
    }
  #elif TEST_MODE == 2
    logInfo("SYSTEM", "TEST_MODE=2 -> menguji semua sensor, WiFi/MQTT tidak dipakai");
    analogSetAttenuation(ADC_11db);
    sensorRain.begin();
    sensorLidar.begin();
    sensorAHT20.begin();
    sensorWind.begin();
    sensorFloat.begin();
    sensorGPS.begin();
    sensorBaterai.begin();

    while (true) {
      testAllSensors();
      delay(TEST_MODE_INTERVAL_MS);
    }
  #endif


  wifiManager.begin();
  mqttManager.begin();

  analogSetAttenuation(ADC_11db);

  sensorRain.begin();
  sensorLidar.begin();
  sensorAHT20.begin();
  sensorWind.begin();
  sensorFloat.begin();
  sensorGPS.begin();
  sensorBaterai.begin();

  lastStatusPrint = millis();
  lastKirimMqtt = millis();
  logInfo("SYSTEM", "Setup selesai. Sistem berjalan.");
}

void loop() {
  // Semua update() di bawah ini NON-BLOCKING: masing-masing modul
  // hanya melakukan pekerjaan bila sudah waktunya, tidak pernah
  // menahan loop() dengan delay() atau while(true).
  wifiManager.update();
  mqttManager.update(wifiManager.isConnected());
  mqttManager.loopClient();

  sensorRain.update();
  sensorLidar.update();
  sensorAHT20.update();
  sensorWind.update();
  sensorFloat.update();
  sensorGPS.update();

  // Kirim data ke MQTT tiap INTERVAL_KIRIM_MQTT_MS, langsung dari sensor asli.
  // Kalau MQTT sedang terputus, publish() di dalamnya hanya akan
  // mencatat log peringatan (data periode itu dilewati), tidak memblokir.
  if (millis() - lastKirimMqtt >= INTERVAL_KIRIM_MQTT_MS) {
    lastKirimMqtt = millis();
    kirimDataKeServer();
  }

  // Heartbeat status berkala, berguna untuk commissioning & debugging lapangan
  if (millis() - lastStatusPrint >= STATUS_PRINT_INTERVAL_MS) {
    ambilDataSensor();
    printSystemStatus();
    lastStatusPrint = millis();
  }
}
