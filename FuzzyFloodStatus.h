/*
  ============================================================
  FuzzyFloodStatus.h - Logika Fuzzy Mamdani untuk Status Banjir
  2 input: TMA (Tinggi Bebas/Freeboard, METER) & Curah Hujan (mm/jam)
  1 output: Skor risiko (0-100) -> label Normal/Siaga/Bahaya

  SUMBER PARAMETER (lihat dokumen "Rancangan_Fuzzy_Logic_Deteksi_Banjir.pdf"
  untuk penjelasan & rujukan lengkap tiap angka):
   - TMA (Freeboard): Kementerian PUPR (2022), Modul 7 Pengelolaan Risiko
     Banjir, Tabel 1.4 "Tingkat Kesiagaan Banjir"
   - Curah Hujan: BMKG - Klasifikasi Intensitas Curah Hujan
   - Rule base: diturunkan dari Pitriyanto & Hariyanto (2024), "Implementasi
     Fuzzy Logic untuk Peringatan Dini Banjir", Multitek Indonesia: Jurnal
     Ilmiah, Vol 18(1), DOI 10.24269/mtkind.v18i1.5912

  CATATAN ARSITEKTUR: modul ini TIDAK menggantikan Float Switch - Float
  Switch tetap jadi crisp override TERPISAH di updateAlarmOutputs()
  (RiverMonitor.ino). Modul ini murni fuzzy logic dari 2 sensor kontinu
  (TMA & Curah Hujan) yang sebelumnya pakai perbandingan >= biasa.
  ============================================================
*/
#ifndef FUZZY_FLOOD_STATUS_H
#define FUZZY_FLOOD_STATUS_H

#include <Arduino.h>
#include "Config.h"

enum class StatusBanjir { NORMAL, SIAGA, BAHAYA };

class FuzzyFloodStatus {
  public:
    // Hitung skor risiko fuzzy (0-100) dari 2 input mentah.
    // freeboardM = Tinggi Bebas dalam METER (BUKAN cm - perhatikan satuan!)
    // curahHujanMmJam = intensitas curah hujan, mm/jam (dari sensorRain.getIntensityMMh())
    float hitungSkor(float freeboardM, float curahHujanMmJam);

    static StatusBanjir skorKeLabel(float skor);
    static const char* labelKeString(StatusBanjir status);

  private:
    static float trapmf(float x, float a, float b, float c, float d);
    static float trimf(float x, float a, float b, float c);
};

#endif // FUZZY_FLOOD_STATUS_H
