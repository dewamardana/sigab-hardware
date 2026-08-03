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
#define TEST_MODE                0
 
// Nama sensor yang diuji kalau TEST_MODE == 1.
// Pilihan: "rain", "lidar", "aht20", "wind", "float", "gps", "bat"
#define TEST_SENSOR_NAME          "rain"
 
// Jeda antar pembacaan saat mode test (ms)
#define TEST_MODE_INTERVAL_MS     500UL

// ---------------- WiFi ----------------
#define WIFI_SSID           "Mardana"
#define WIFI_PASSWORD       "Qwerty123@"
#define WIFI_CONNECT_TIMEOUT_MS   15000UL   // batas waktu 1x percobaan konek
#define WIFI_RETRY_INTERVAL_MS    5000UL    // jeda antar percobaan reconnect

// ---------------- MQTT ----------------
#define MQTT_HOST                 "103.171.85.108"
#define MQTT_PORT                 1883
#define MQTT_CLIENT_ID            "B001"
#define TOPIC_SENSOR              "sigab/bali/sensor"
#define TOPIC_STATUS              "sigab/bali/status"
#define TOPIC_ONLINE              "sigab/bali/online"
#define MQTT_RETRY_INTERVAL_MS    5000UL   // jeda antar percobaan connect, non-blocking
#define MQTT_BUFFER_SIZE          256
#define INTERVAL_KIRIM_MQTT_MS    30000UL  // seberapa sering data dikirim ke server
#define MQTT_USER   "sigab_device"
#define MQTT_PASS   "A#eGa936@CCbf26"

// ---------------- Baterai - INA226 (I2C) ----------------
#define INA226_I2C_ADDRESS         0x40    // A1=A0=GND (Table 6-2 datasheet). Sesuaikan kalau beda.
#define INA226_READ_INTERVAL_MS    2000UL  // seberapa sering memicu konversi baru
#define INA226_CONVERSION_WAIT_MS  5UL     // jeda non-blocking sebelum baca hasil (margin di atas waktu konversi ~1.1-1.21ms)
#define INA226_RETRY_INTERVAL_MS   3000UL  // jeda percobaan re-init saat unhealthy
#define INA226_MAX_FAIL_COUNT      5       // gagal beruntun sebelum dianggap unhealthy

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

// Referensi tinggi pemasangan sensor (cm) - HANYA dipakai untuk field
// "tma_cm" (kompatibilitas dashboard/Node-RED yang sudah ada, gaya lama).
// TIDAK dipakai untuk keputusan status/alarm apa pun - itu sepenuhnya
// pakai JARAK_SENSOR_KE_TEBING_KRITIS_CM + freeboard di blok "Logika
// Fuzzy" di bawah. Dua konstanta ini SENGAJA terpisah karena mengukur
// jarak ke 2 titik referensi yang beda (dasar sungai vs tebing kritis) -
// lihat FUZZY_LOGIC_README.md bagian "tma_cm vs freeboard_m" utk detail.
#define TFLUNA_TINGGI_PEMASANGAN_CM   300.0f  // <-- sesuaikan/isi sesuai kondisi lapangan

// ---------------- AHT20 (Suhu & Kelembapan) ----------------
#define AHT20_READ_INTERVAL_MS   2000UL
#define AHT20_RETRY_INTERVAL_MS  3000UL
#define AHT20_MAX_FAIL_COUNT     5

// ---------------- OLED Display (I2C, SSD1306) ----------------
// Layar OLED berbagi bus I2C yang SAMA dengan TF-Luna & AHT20
// (I2C_SDA/I2C_SCL di atas) - aman satu bus karena alamatnya beda.
//
// 0 = OFF    -> OLED tidak diinisialisasi/dipakai sama sekali
// 1 = DEBUG  -> tampilkan data MENTAH semua sensor + status koneksi,
//               digilir 3 halaman (banyak info -> cocok dipakai
//               teknisi saat instalasi/troubleshooting di lapangan)
// 2 = NORMAL -> tampilkan ringkasan penting saja dalam satu layar
//               (TMA, baterai, kategori hujan, status WiFi/MQTT) -
//               cocok untuk pemantauan harian oleh operator
// Ganti angka ini saja untuk pindah mode - TIDAK perlu edit .ino.
#define OLED_MODE                  1

#define OLED_I2C_ADDRESS           0x3C   // umum utk modul SSD1306 0.96" 128x64. Coba 0x3D kalau tidak terdeteksi
#define OLED_WIDTH                 128
#define OLED_HEIGHT                 64
#define OLED_RESET_PIN               -1   // -1 = tidak pakai pin reset terpisah (umum utk modul I2C 4-pin)
#define OLED_UPDATE_INTERVAL_MS     500UL // jeda refresh layar (jangan terlalu cepat -> flicker/beban I2C)
#define OLED_DEBUG_PAGE_INTERVAL_MS 2000UL // di mode DEBUG, halaman gantian tiap sekian ms
#define OLED_RETRY_INTERVAL_MS      5000UL // jeda percobaan re-init kalau OLED tidak terdeteksi


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

// ---------------- Output Alarm: Buzzer & Flash (modul MOSFET IRF520) ----------------
// Tiap beban disambung lewat 1 modul IRF520 (SIG ke GPIO di bawah, beban
// diberi CATU DAYA TERPISAH lewat V+/V-/OUT modul - lihat MosfetSwitch.h
// utk detail wiring). Sesuaikan nomor pin ini dengan GPIO yang benar-benar
// kamu pakai di board (belum dipakai sensor lain).
#define PIN_BUZZER                1 // pin 1 - Buzzer BJ-1KL via IRF520
#define PIN_FLASH                 2  // pin 2 - Flash/senter via IRF520 (trigger-nya menyusul)
#define BUZZER_ACTIVE_HIGH        true // true = SIG HIGH menyalakan MOSFET (umum utk modul IRF520)
#define FLASH_ACTIVE_HIGH         true

// ---------------- Trigger ESP32-CAM (UART2 hardware) ----------------
// UART2 - masih sepenuhnya bebas di ESP32-S3 - di-route ke pin
// pilihan sendiri lewat GPIO Matrix (lihat EspCamTrigger.cpp).
#define PIN_CAM_RX                11   // RX ESP32-S3 (dari TX ESP32-CAM, opsional dipakai)
#define PIN_CAM_TX                10   // TX ESP32-S3 (ke RX ESP32-CAM) - jalur perintah capture
#define CAM_BAUD                  115200 // UART hardware andal di baud tinggi - SESUAIKAN dgn baud firmware ESP32-CAM
#define CAM_CAPTURE_CMD           "CAPTURE\n" // perintah dikirim - firmware ESP32-CAM harus mengenali ini
// Jeda non-blocking: flash dinyalakan dulu, BARU setelah CAM_FLASH_WARMUP_MS
// berlalu perintah capture dikirim - supaya foto tidak gelap.
#define CAM_FLASH_WARMUP_MS       300UL

// ---------------- Umum ----------------
#define STATUS_PRINT_INTERVAL_MS 1000UL

// ---------------- Logika Fuzzy: Deteksi Dini Status Banjir ----------------
// Lihat dokumen "Rancangan_Fuzzy_Logic_Deteksi_Banjir.pdf" dan
// "FUZZY_LOGIC_README.md" utk penjelasan & sumber lengkap tiap parameter.
// SATU-SATUNYA sumber status Siaga/Bahaya dari sensor kontinu (TMA+hujan) -
// MENGGANTIKAN TOTAL logika crisp lama (TMA_SIAGA_CM/TMA_BAHAYA_CM).

// --- Kalibrasi lapangan (WAJIB diisi manual setelah survei fisik!) ---
// Jarak dari sensor TF-Luna ke titik tebing/tanggul KRITIS (BUKAN ke dasar
// sungai) - diukur SATU KALI secara manual di lokasi pemasangan.
#define JARAK_SENSOR_KE_TEBING_KRITIS_CM   300.0f  // <-- GANTI dgn hasil ukur nyata!

// --- Variabel TMA (Tinggi Bebas/Freeboard), satuan METER ---
// Sumber: Kementerian PUPR (2022), Modul 7 Pengelolaan Risiko Banjir, Tabel 1.4
#define FUZZY_TMA_BAHAYA_A   0.00f
#define FUZZY_TMA_BAHAYA_B   0.00f
#define FUZZY_TMA_BAHAYA_C   0.75f
#define FUZZY_TMA_BAHAYA_D   1.20f
#define FUZZY_TMA_SIAGA_A    0.75f
#define FUZZY_TMA_SIAGA_B    1.20f
#define FUZZY_TMA_SIAGA_C    1.50f
#define FUZZY_TMA_NORMAL_A   1.20f
#define FUZZY_TMA_NORMAL_B   1.50f
#define FUZZY_TMA_NORMAL_C   2.50f   // UoD_max - SESUAIKAN dgn kondisi lapangan
#define FUZZY_TMA_NORMAL_D   2.50f

// --- Variabel Curah Hujan, satuan mm/jam ---
// Sumber: BMKG - Klasifikasi Intensitas Curah Hujan
#define FUZZY_HUJAN_RINGAN_A   0.0f
#define FUZZY_HUJAN_RINGAN_B   0.0f
#define FUZZY_HUJAN_RINGAN_C   10.0f
#define FUZZY_HUJAN_RINGAN_D   20.0f
#define FUZZY_HUJAN_SEDANG_A   10.0f
#define FUZZY_HUJAN_SEDANG_B   20.0f
#define FUZZY_HUJAN_SEDANG_C   50.0f
#define FUZZY_HUJAN_LEBAT_A    20.0f
#define FUZZY_HUJAN_LEBAT_B    50.0f
#define FUZZY_HUJAN_LEBAT_C    150.0f
#define FUZZY_HUJAN_LEBAT_D    150.0f

// --- Variabel Output (skor risiko internal, skala 0-100) ---
#define FUZZY_OUT_AMAN_A     0.0f
#define FUZZY_OUT_AMAN_B     0.0f
#define FUZZY_OUT_AMAN_C     20.0f
#define FUZZY_OUT_AMAN_D     40.0f
#define FUZZY_OUT_SIAGA_A    20.0f
#define FUZZY_OUT_SIAGA_B    50.0f
#define FUZZY_OUT_SIAGA_C    50.0f
#define FUZZY_OUT_SIAGA_D    80.0f
#define FUZZY_OUT_BAHAYA_A   60.0f
#define FUZZY_OUT_BAHAYA_B   80.0f
#define FUZZY_OUT_BAHAYA_C   100.0f
#define FUZZY_OUT_BAHAYA_D   100.0f

// --- Ambang label akhir setelah defuzzifikasi ---
#define FUZZY_SKOR_BATAS_NORMAL_SIAGA   35.0f
#define FUZZY_SKOR_BATAS_SIAGA_BAHAYA   65.0f

// --- Resolusi integrasi numerik centroid ---
#define FUZZY_CENTROID_STEP   1.0f

#endif // CONFIG_H
