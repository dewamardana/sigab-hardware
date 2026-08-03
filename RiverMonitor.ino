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
    SensorAnalog.*       -> [tidak dipakai lagi] voltage divider baterai, digantikan SensorINA226
    SensorINA226.*       -> tegangan baterai (I2C)
    MosfetSwitch.*       -> saklar ON/OFF generik utk modul IRF520 (Buzzer & Flash)
    EspCamTrigger.*      -> trigger ESP32-CAM ambil gambar via UART2
    FuzzyFloodStatus.*   -> logika fuzzy Mamdani utk status Normal/Siaga/Bahaya
                            (SATU-SATUNYA sumber status - lihat FUZZY_LOGIC_README.md)

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
#include "DisplayOLED.h"
#include "SensorINA226.h"
#include "MosfetSwitch.h"
#include "EspCamTrigger.h"
#include "FuzzyFloodStatus.h"

WiFiManager wifiManager;
MqttManager mqttManager;
DisplayOLED displayOLED;
SensorRain sensorRain;
SensorLidar sensorLidar;
SensorAHT20 sensorAHT20;
SensorWind sensorWind;
SensorFloatSwitch sensorFloat;
SensorGPS sensorGPS;
SensorINA226 sensorINA226;
EspCamTrigger camTrigger;
MosfetSwitch buzzerSwitch(PIN_BUZZER, BUZZER_ACTIVE_HIGH);
MosfetSwitch flashSwitch(PIN_FLASH, FLASH_ACTIVE_HIGH);
FuzzyFloodStatus fuzzyFlood;


RiverData data;

unsigned long lastStatusPrint = 0;
unsigned long lastKirimMqtt = 0;
bool alarmTmaSebelumnya = false;  // utk deteksi transisi naik (edge) alarm

// Cache hasil fuzzy TERKINI - dihitung SEKALI per loop() di updateAlarmOutputs(),
// dibaca ulang oleh ambilDataSensor() (BUKAN dihitung ulang) supaya tidak ada
// duplikasi logika status di lebih dari satu tempat.
float skorFuzzyTerkini = 0.0f;
StatusBanjir statusFuzzyTerkini = StatusBanjir::NORMAL;

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
    // DUA perhitungan terpisah dari 1 pembacaan sensor, referensi beda:
    data.tma_cm      = TFLUNA_TINGGI_PEMASANGAN_CM - sensorLidar.getDistanceCM(); // legacy, kompatibilitas dashboard - TIDAK dipakai fuzzy
    data.freeboard_m = (JARAK_SENSOR_KE_TEBING_KRITIS_CM - sensorLidar.getDistanceCM()) / 100.0f; // dipakai fuzzy/status
  }

  if (sensorINA226.isHealthy()) {
    data.baterai_v = sensorINA226.getBusVoltageV();
  }

  data.angin_kmph = sensorWind.getSpeedKMH();
  data.hujan_mm = sensorRain.getTotalMM();
  data.hujan_intensitas_mmh = sensorRain.getIntensityMMh();
  data.hujan_kategori = sensorRain.getCategory();
  data.levelKritis = sensorFloat.isWaterHigh();

  // Status fuzzy - SATU-SATUNYA sumber (dihitung di updateAlarmOutputs(),
  // di sini cuma DIBACA dari cache, tidak dihitung ulang).
  data.statusSkor = skorFuzzyTerkini;
  data.statusLabel = FuzzyFloodStatus::labelKeString(statusFuzzyTerkini);

  data.gpsFix = sensorGPS.hasFix();
  if (data.gpsFix) {
    data.gpsLat = sensorGPS.getLat();
    data.gpsLng = sensorGPS.getLng();
  }
}

// ------------------------------------------------------------
// Sinkronkan output alarm & picu kamera berdasarkan LOGIKA FUZZY (Mamdani)
// dari 2 sensor kontinu: TMA (freeboard) & Curah Hujan. Ini SATU-SATUNYA
// tempat status Normal/Siaga/Bahaya dihitung - lihat FuzzyFloodStatus.h/.cpp
// dan FUZZY_LOGIC_README.md untuk penjelasan lengkap & sumber tiap parameter.
//
//   BAHAYA = Float Switch AKTIF  ATAU  status fuzzy == BAHAYA
//            -> BUZZER menyala (HANYA di kondisi ini)
//   SIAGA  = status fuzzy == SIAGA (atau BAHAYA)
//            -> bersama BAHAYA, FLASH & KAMERA menyala (kedua kondisi)
//
// Dipanggil TIAP loop() - langsung pakai getter sensor real-time (BUKAN
// data.freeboard_m/data.levelKritis yang cuma di-refresh tiap
// ambilDataSensor() dipanggil, bisa telat ~1 detik).
// ------------------------------------------------------------
void updateAlarmOutputs() {
  bool airTinggi = sensorFloat.isWaterHigh(); // Float Switch - crisp override, TETAP di luar fuzzy

  float freeboardM = (JARAK_SENSOR_KE_TEBING_KRITIS_CM - sensorLidar.getDistanceCM()) / 100.0f;
  bool  lidarOk     = sensorLidar.isHealthy();

  if (lidarOk) {
    skorFuzzyTerkini   = fuzzyFlood.hitungSkor(freeboardM, sensorRain.getIntensityMMh());
    statusFuzzyTerkini = FuzzyFloodStatus::skorKeLabel(skorFuzzyTerkini);
  }
  // Kalau !lidarOk, skorFuzzyTerkini/statusFuzzyTerkini SENGAJA dibiarkan
  // nilai lama - konsisten dgn filosofi "jangan timpa dgn 0 saat sensor
  // unhealthy" yang dipakai di seluruh project.

  bool siagaDariFuzzy  = lidarOk && (statusFuzzyTerkini == StatusBanjir::SIAGA || statusFuzzyTerkini == StatusBanjir::BAHAYA);
  bool bahayaDariFuzzy = lidarOk && (statusFuzzyTerkini == StatusBanjir::BAHAYA);

  bool bahaya          = airTinggi || bahayaDariFuzzy; // BUZZER: HANYA kondisi ini
  bool siagaAtauBahaya = siagaDariFuzzy || bahaya;     // FLASH & KAMERA: kedua kondisi ini

  buzzerSwitch.set(bahaya);
  flashSwitch.set(siagaAtauBahaya);

  // Deteksi transisi naik (baru saja MASUK status siaga/bahaya) - picu
  // SEKALI saja per kejadian, bukan tiap loop() selama status masih aktif.
  if (siagaAtauBahaya && !alarmTmaSebelumnya) {
    logWarn("SYSTEM", "!!! STATUS %s (skor fuzzy=%.1f, freeboard=%.2fm, hujan=%.1fmm/jam%s) - flash menyala, memicu ESP32-CAM !!!",
            bahaya ? "BAHAYA" : "SIAGA", skorFuzzyTerkini, freeboardM, sensorRain.getIntensityMMh(),
            airTinggi ? ", FloatSwitch-AKTIF" : "");
    // Flash SUDAH dinyalakan di atas (flashSwitch.set(true)) SEBELUM baris
    // ini - camTrigger.requestCapture() hanya menandai permintaan, lalu
    // camTrigger.update() (dipanggil tiap loop()) yang benar-benar mengirim
    // perintah setelah CAM_FLASH_WARMUP_MS berlalu (non-blocking).
    camTrigger.requestCapture();
  } else if (!siagaAtauBahaya && alarmTmaSebelumnya) {
    logInfo("SYSTEM", "Status kembali NORMAL.");
  }

  alarmTmaSebelumnya = siagaAtauBahaya;
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
  logInfo("SYSTEM", "TF-Luna     : %s | jarak=%d cm, tma=%.1f cm (legacy), freeboard=%.2f m, sinyal=%d",
          sensorLidar.isHealthy() ? "OK" : "ERROR",
          sensorLidar.getDistanceCM(), data.tma_cm, data.freeboard_m, sensorLidar.getStrength());
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
  logInfo("SYSTEM", "Status Fuzzy: %s (skor=%.1f) | Buzzer: %s | Flash: %s",
          data.statusLabel.c_str(), data.statusSkor,
          buzzerSwitch.isOn() ? "MENYALA" : "mati",
          flashSwitch.isOn() ? "MENYALA" : "mati");
  logInfo("SYSTEM", "========================================================");
}

void setup() {
  Serial.begin(115200);
  delay(300);

  logInfo("SYSTEM", "=== River Monitoring System - Booting ===");
  displayOLED.begin();  // aman dipanggil duluan - tidak bentrok dgn sensor I2C lain (alamat beda)

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
  sensorINA226.begin();

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
  sensorINA226.begin();
  buzzerSwitch.begin();
  flashSwitch.begin();
  camTrigger.begin();

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
  updateAlarmOutputs();
  sensorINA226.update();
  camTrigger.update();

  // Kirim data ke MQTT tiap INTERVAL_KIRIM_MQTT_MS, langsung dari sensor asli.
  // Kalau MQTT sedang terputus, publish() di dalamnya hanya akan
  // mencatat log peringatan (data periode itu dilewati), tidak memblokir.
  if (millis() - lastKirimMqtt >= INTERVAL_KIRIM_MQTT_MS) {
    lastKirimMqtt = millis();
    kirimDataKeServer();
  }

  // Tampilan OLED - non-blocking, throttled sendiri lewat OLED_UPDATE_INTERVAL_MS.
  // Kontennya (DEBUG/NORMAL) tergantung OLED_MODE di Config.h.
  displayOLED.update(data, wifiManager.isConnected(), mqttManager.isConnected());

  // Heartbeat status berkala, berguna untuk commissioning & debugging lapangan
  if (millis() - lastStatusPrint >= STATUS_PRINT_INTERVAL_MS) {
    ambilDataSensor();
    printSystemStatus();
    lastStatusPrint = millis();
  }
}
