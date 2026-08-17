# 🌊 SIGAB Hardware — Firmware Monitoring Sungai/Banjir

Kode firmware (ESP32/Arduino) untuk perangkat keras sistem **SIGAB**, dipasangkan dengan dashboard web di repo [`sigab-dashboard`](https://github.com/dewamardana/sigab-dashboard).

## 📌 Deskripsi

Firmware modular untuk unit pemantauan sungai berbasis mikrokontroler. Membaca berbagai sensor lingkungan, menghitung status risiko banjir menggunakan **logika fuzzy**, lalu mengirim data ke server melalui **MQTT**.

## ⚙️ Modul / Fitur Utama

| Modul | Fungsi |
|---|---|
| `RiverMonitor.ino` | Program utama |
| `SensorRain.*` | Sensor curah hujan |
| `SensorWind.*` | Sensor kecepatan angin |
| `SensorAHT20.*` | Sensor suhu & kelembapan |
| `SensorFloatSwitch.*` | Sensor pelampung (ketinggian air) |
| `SensorLidar.*` | Sensor jarak/ketinggian LiDAR |
| `SensorGPS.*` | Lokasi perangkat |
| `SensorAnalog.*` | Sensor analog tambahan |
| `SensorINA226.*` | Monitoring daya/baterai |
| `FuzzyFloodStatus.*` | Logika fuzzy penentu status banjir |
| `EspCamTrigger.*` | Pemicu kamera ESP32-CAM |
| `MqttManager.*` | Pengiriman data via MQTT |
| `WiFiManager.*` | Manajemen koneksi WiFi |
| `DisplayOLED.*` | Tampilan OLED lokal |
| `JsonBuilder.*` | Penyusun payload JSON |

## ⚙️ Teknologi

- **Platform:** ESP32 (Arduino framework)
- **Protokol:** MQTT, WiFi
- **Metode:** Logika Fuzzy untuk klasifikasi status banjir

## 🚀 Cara Menjalankan

1. Buka `RiverMonitor.ino` di **Arduino IDE** / PlatformIO.
2. Sesuaikan kredensial WiFi & broker MQTT di `Config.h`.
3. Upload ke board ESP32.

## 🔗 Proyek Terkait

- [`sigab-dashboard`](https://github.com/dewamardana/sigab-dashboard) — dashboard Laravel yang menampilkan data dari perangkat ini.
