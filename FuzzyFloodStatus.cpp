#include "FuzzyFloodStatus.h"

float FuzzyFloodStatus::trapmf(float x, float a, float b, float c, float d)
{
  // PENTING: cek plateau (b<=x<=c) LEBIH DULU. Kalau urutan dibalik (cek
  // x<=a duluan), kasus trapesium "rata mulai dari titik awal" (a==b, mis.
  // trapmf(x,0,0,10,20)) akan SALAH dianggap 0 tepat di titik a==b - ini
  // bug nyata yang sempat ketemu & diperbaiki saat pengujian standalone
  // sebelum kode ini ditulis (lihat riwayat pengujian).
  if (x >= b && x <= c) return 1.0f;
  if (x > a && x < b)   return (x - a) / (b - a);
  if (x > c && x < d)   return (d - x) / (d - c);
  return 0.0f;
}

float FuzzyFloodStatus::trimf(float x, float a, float b, float c)
{
  return trapmf(x, a, b, b, c);
}

float FuzzyFloodStatus::hitungSkor(float freeboardM, float curahHujanMmJam)
{
  // ---------- 1) FUZZIFIKASI ----------
  float muTmaBahaya = trapmf(freeboardM, FUZZY_TMA_BAHAYA_A, FUZZY_TMA_BAHAYA_B,
                              FUZZY_TMA_BAHAYA_C, FUZZY_TMA_BAHAYA_D);
  float muTmaSiaga  = trimf(freeboardM, FUZZY_TMA_SIAGA_A, FUZZY_TMA_SIAGA_B, FUZZY_TMA_SIAGA_C);
  float muTmaNormal = trapmf(freeboardM, FUZZY_TMA_NORMAL_A, FUZZY_TMA_NORMAL_B,
                              FUZZY_TMA_NORMAL_C, FUZZY_TMA_NORMAL_D);

  float muHujanRingan = trapmf(curahHujanMmJam, FUZZY_HUJAN_RINGAN_A, FUZZY_HUJAN_RINGAN_B,
                                FUZZY_HUJAN_RINGAN_C, FUZZY_HUJAN_RINGAN_D);
  float muHujanSedang = trimf(curahHujanMmJam, FUZZY_HUJAN_SEDANG_A, FUZZY_HUJAN_SEDANG_B,
                               FUZZY_HUJAN_SEDANG_C);
  float muHujanLebat  = trapmf(curahHujanMmJam, FUZZY_HUJAN_LEBAT_A, FUZZY_HUJAN_LEBAT_B,
                                FUZZY_HUJAN_LEBAT_C, FUZZY_HUJAN_LEBAT_D);

  // ---------- 2) INFERENSI (rule base 3x3, operator AND = min) ----------
  // Rule base BARU: TMA (freeboard, PUPR 2022) jadi BASELINE OTORITATIF -
  // kolom TMA Siaga/Bahaya SELALU menghasilkan minimal Siaga/Bahaya, TIDAK
  // PERNAH diturunkan oleh hujan ringan/tidak ada hujan. Curah hujan HANYA
  // berperan MENGESKALASI (menaikkan 1 tingkat), tidak pernah menurunkan -
  // konsisten dengan metode resmi PUPR "hubungan curah hujan-TMA" sebagai
  // faktor modulasi, bukan penentu utama pengganti freeboard.
  //              TMA Normal      TMA Siaga       TMA Bahaya
  // Hujan Ringan  Aman  (R1)      Siaga (R2)      Bahaya (R3)
  // Hujan Sedang  Aman  (R4)      Siaga (R5)      Bahaya (R6)
  // Hujan Lebat   Siaga (R7)      Bahaya(R8)      Bahaya (R9)
  float r1 = min(muHujanRingan, muTmaNormal);
  float r2 = min(muHujanRingan, muTmaSiaga);
  float r3 = min(muHujanRingan, muTmaBahaya);
  float r4 = min(muHujanSedang, muTmaNormal);
  float r5 = min(muHujanSedang, muTmaSiaga);
  float r6 = min(muHujanSedang, muTmaBahaya);
  float r7 = min(muHujanLebat,  muTmaNormal);
  float r8 = min(muHujanLebat,  muTmaSiaga);
  float r9 = min(muHujanLebat,  muTmaBahaya);

  // ---------- 3) AGREGASI (union/max tiap label output) ----------
  float alphaAman   = max(r1, r4);
  float alphaSiaga  = max(r2, max(r5, r7));
  float alphaBahaya = max(r3, max(r6, max(r8, r9)));

  // ---------- 4) DEFUZZIFIKASI (Centroid, integrasi numerik) ----------
  float pembilang = 0.0f;
  float penyebut  = 0.0f;

  for (float y = 0.0f; y <= 100.0f; y += FUZZY_CENTROID_STEP)
  {
    float muAman   = min(alphaAman,   trapmf(y, FUZZY_OUT_AMAN_A, FUZZY_OUT_AMAN_B,
                                              FUZZY_OUT_AMAN_C, FUZZY_OUT_AMAN_D));
    float muSiaga  = min(alphaSiaga,  trapmf(y, FUZZY_OUT_SIAGA_A, FUZZY_OUT_SIAGA_B,
                                              FUZZY_OUT_SIAGA_C, FUZZY_OUT_SIAGA_D));
    float muBahaya = min(alphaBahaya, trapmf(y, FUZZY_OUT_BAHAYA_A, FUZZY_OUT_BAHAYA_B,
                                              FUZZY_OUT_BAHAYA_C, FUZZY_OUT_BAHAYA_D));

    float muGabungan = max(muAman, max(muSiaga, muBahaya));

    pembilang += y * muGabungan;
    penyebut  += muGabungan;
  }

  if (penyebut == 0.0f)
  {
    return 0.0f; // seharusnya tidak terjadi kalau UoD sudah benar - jaga-jaga saja
  }

  return pembilang / penyebut;
}

StatusBanjir FuzzyFloodStatus::skorKeLabel(float skor)
{
  if (skor < FUZZY_SKOR_BATAS_NORMAL_SIAGA) return StatusBanjir::NORMAL;
  if (skor < FUZZY_SKOR_BATAS_SIAGA_BAHAYA) return StatusBanjir::SIAGA;
  return StatusBanjir::BAHAYA;
}

const char* FuzzyFloodStatus::labelKeString(StatusBanjir status)
{
  switch (status)
  {
    case StatusBanjir::NORMAL: return "NORMAL";
    case StatusBanjir::SIAGA:  return "SIAGA";
    case StatusBanjir::BAHAYA: return "BAHAYA";
  }
  return "UNKNOWN";
}
