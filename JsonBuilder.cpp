#include "JsonBuilder.h"
#include <ArduinoJson.h>
#include "Config.h"

String buatJSON(const RiverData &data)
{
  JsonDocument doc;
  doc["device"]           = MQTT_CLIENT_ID;
  doc["suhu"]             = round(data.suhu               * 10)  / 10.0;
  doc["kelembapan"]       = round(data.kelembapan         * 10)  / 10.0;
  doc["tma_cm"]           = round(data.tma_cm             * 10)  / 10.0; // kompatibilitas dashboard lama
  doc["freeboard_m"]      = round(data.freeboard_m        * 100) / 100.0; // dipakai fuzzy/status
  doc["angin_kmph"]       = round(data.angin_kmph         * 10)  / 10.0;
  doc["baterai_v"]        = round(data.baterai_v          * 100) / 100.0;
  doc["hujan_mm"]         = round(data.hujan_mm           * 10)  / 10.0;
  doc["hujan_intensitas"] = round(data.hujan_intensitas_mmh * 10) / 10.0;
  doc["hujan_kategori"]   = data.hujan_kategori;
  doc["level_kritis"]     = data.levelKritis;
  // BARU - status fuzzy dihitung PENUH di perangkat (bukan lagi di-derive
  // ulang di server/Node-RED dari ambang crisp terpisah) - SATU-SATUNYA
  // sumber kebenaran status Normal/Siaga/Bahaya, lihat FuzzyFloodStatus.
  doc["status_skor"]      = round(data.statusSkor * 10) / 10.0;
  doc["status_label"]     = data.statusLabel;

  if (data.gpsFix)
  {
    doc["lat"] = data.gpsLat;
    doc["lng"] = data.gpsLng;
  }

  String output;
  serializeJson(doc, output);
  return output;
}
