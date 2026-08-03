/*
  ============================================================
  RiverData.h
  Struct tunggal berisi snapshot semua nilai sensor terkini,
  diisi ulang tiap siklus kirim dari getter masing-masing modul
  sensor, lalu dipakai bersama oleh JsonBuilder dan tampilan status.
  ============================================================
*/
#ifndef RIVER_DATA_H
#define RIVER_DATA_H

#include <Arduino.h>

struct RiverData {
  float suhu           = 0.0f;   // C, dari AHT20
  float kelembapan     = 0.0f;   // %RH, dari AHT20
  float tma_cm         = 0.0f;   // KOMPATIBILITAS dashboard lama - TIDAK dipakai fuzzy/status
  float freeboard_m    = 0.0f;   // dipakai fuzzy/status - jarak muka air ke tebing kritis (meter)
  float angin_kmph     = 0.0f;   // dari anemometer
  float baterai_v      = 0.0f;   // dari INA226
  float hujan_mm       = 0.0f;   // akumulasi total curah hujan (mm)
  float hujan_intensitas_mmh = 0.0f; // intensitas real-time (mm/jam)
  String hujan_kategori = "Tidak ada hujan";
  bool  levelKritis    = false;  // dari float switch
  float statusSkor     = 0.0f;   // skor risiko fuzzy (0-100), lihat FuzzyFloodStatus
  String statusLabel   = "NORMAL"; // label fuzzy: NORMAL/SIAGA/BAHAYA
  bool  gpsFix         = false;
  double gpsLat        = 0.0;
  double gpsLng        = 0.0;
};

#endif // RIVER_DATA_H
