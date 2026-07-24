/*
  ============================================================
  Config.h
  Semua pin, konstanta kalibrasi, dan parameter timing
  dikumpulkan di sini supaya mudah diubah tanpa menyentuh
  logika program di file lain.
  ============================================================
*/
#ifndef CONFIG_H
#define CONFIG_H

// ---------------- Mode Pengujian Sensor (SensorTest.h) ----------------
// 0 = Mode normal (WiFi + MQTT jalan seperti biasa, produksi)
// 1 = Uji SATU sensor saja (lihat TEST_SENSOR_NAME di bawah), tanpa WiFi/MQTT
// 2 = Uji SEMUA sensor, tampilkan payload JSON yg akan dikirim, tanpa WiFi/MQTT
// Ganti angka ini saja untuk pindah mode - TIDAK perlu edit/comment .ino.
#define TEST_MODE                1
 
// Nama sensor yang diuji kalau TEST_MODE == 1.
// Pilihan: "rain", "lidar", "aht20", "wind", "float", "gps", "bat"
#define TEST_SENSOR_NAME          "rain"
 
// Jeda antar pembacaan saat mode test (ms)
#define TEST_MODE_INTERVAL_MS     500UL

// ---------------- WiFi ----------------
#define WIFI_SSID           "iPhone"
#define WIFI_PASSWORD       "45454545"
#define WIFI_CONNECT_TIMEOUT_MS   15000UL   // batas waktu 1x percobaan konek
#define WIFI_RETRY_INTERVAL_MS    5000UL    // jeda antar percobaan reconnect

// ---------------- MQTT ----------------
#define MQTT_HOST                 "10.109.234.99"
#define MQTT_PORT                 1883
#define MQTT_CLIENT_ID            "B004"
#define TOPIC_SENSOR              "sigab/bali/sensor"
#define TOPIC_STATUS              "sigab/bali/status"
#define TOPIC_ONLINE              "sigab/bali/online"
#define MQTT_RETRY_INTERVAL_MS    5000UL   // jeda antar percobaan connect, non-blocking
#define MQTT_BUFFER_SIZE          256
#define INTERVAL_KIRIM_MQTT_MS    10000UL  // seberapa sering data dikirim ke server

// ---------------- Baterai (voltage divider ke pin ADC) ----------------
#define PIN_POT_BATERAI           4       // sesuaikan dengan pin ADC yang dipakai voltage divider
#define BATERAI_MIN_V             10.0f   // tegangan pada pembacaan ADC 0
#define BATERAI_RENTANG_V         4.0f    // rentang tambahan di atas BATERAI_MIN_V pada ADC penuh (3300 mV)

// ---------------- Debug / Logging ----------------
// 0 = OFF, 1 = ERROR, 2 = WARN, 3 = INFO, 4 = DEBUG
#define LOG_LEVEL           4

// ---------------- Rain Gauge (Tipping Bucket) ----------------
#define PIN_RAIN_INTERRUPT      7       // GPIO ESP32-S3, hindari strapping pin (0,3,45,46)
#define RAIN_MM_PER_TIP         0.70f   // hasil kalibrasi vendor
#define RAIN_DEBOUNCE_US        50000UL
#define RAIN_WINDOW_MS          10000UL // jendela hitung intensitas: 1 menit (ubah ke 30000 utk 30 detik, dst)

// ---------------- TF-Luna LiDAR (I2C - ketinggian air) ----------------
#define I2C_SDA                 8
#define I2C_SCL                 9
#define TFLUNA_I2C_ADDRESS      0x10
#define TFLUNA_DATA_LENGTH      9
#define TFLUNA_READ_INTERVAL_MS 100UL
#define TFLUNA_MAX_FAIL_COUNT   5      // gagal beruntun sebelum dianggap unhealthy
#define TFLUNA_RETRY_INTERVAL_MS 3000UL // jeda percobaan re-init saat unhealthy

// Tinggi sensor terpasang di atas titik referensi/dasar sungai (cm).
// Tinggi muka air (tma_cm) = TFLUNA_TINGGI_PEMASANGAN_CM - jarak terbaca sensor.
// WAJIB diukur & disesuaikan saat pemasangan di lapangan.
#define TFLUNA_TINGGI_PEMASANGAN_CM   300.0f

// ---------------- AHT20 (Suhu & Kelembapan) ----------------
#define AHT20_READ_INTERVAL_MS   2000UL
#define AHT20_RETRY_INTERVAL_MS  3000UL
#define AHT20_MAX_FAIL_COUNT     5

// ---------------- Anemometer (Kecepatan Angin) ----------------
#define PIN_WIND_INTERRUPT       6
#define WIND_DEBOUNCE_US         5000UL
#define WIND_WINDOW_MS           10000UL
#define WIND_MIN_MS              1.5f    // minimum valid reading (m/s)

// ---------------- Float Switch (Level Air Kritis) ----------------
#define PIN_FLOAT_SWITCH         5
#define FLOAT_DEBOUNCE_MS        50UL

// ---------------- GPS ----------------
#define GPS_RXD2                 18
#define GPS_TXD2                 17
#define GPS_BAUD                 9600
#define GPS_STALE_TIMEOUT_MS     10000UL  // tanpa karakter masuk = GPS dianggap unhealthy

// ---------------- Umum ----------------
#define STATUS_PRINT_INTERVAL_MS 1000UL

#endif // CONFIG_H
