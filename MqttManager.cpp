#include "MqttManager.h"
#include "Logger.h"

void MqttManager::begin()
{
  _mqtt.setServer(MQTT_HOST, MQTT_PORT);
  _mqtt.setBufferSize(MQTT_BUFFER_SIZE);
  _lastAttemptAt = 0; // supaya percobaan pertama langsung jalan
}

void MqttManager::tryConnect()
{
  logInfo("MQTT", "Menghubungkan ke %s:%d sebagai '%s' ...", MQTT_HOST, MQTT_PORT, MQTT_USER);

  // BARU - kirim username & password, bukan lagi connect() polos.
  // Mosquitto di VPS akan menolak koneksi tanpa ini (kode state 5).
  if (_mqtt.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS))
  {
    _state = MQTT_STATE_CONNECTED;
    logInfo("MQTT", "Terhubung ke broker.");
    _mqtt.publish(TOPIC_ONLINE, "{\"status\":\"online\"}");
  }
  else
  {
    _state = MQTT_STATE_DISCONNECTED;
    logWarn("MQTT", "Gagal connect, kode state: %d (cek lagi MQTT_USER/MQTT_PASS kalau kodenya 5 = tidak diotorisasi). Coba lagi dalam %lu ms",
            _mqtt.state(), MQTT_RETRY_INTERVAL_MS);
  }
  _lastAttemptAt = millis();
}

void MqttManager::update(bool wifiConnected)
{
  if (!wifiConnected)
  {
    // Tidak ada gunanya mencoba MQTT kalau WiFi sendiri belum tersambung.
    // WiFiManager yang menangani reconnect WiFi secara terpisah.
    if (_state == MQTT_STATE_CONNECTED)
    {
      logWarn("MQTT", "WiFi terputus, MQTT ikut dianggap terputus.");
    }
    _state = MQTT_STATE_DISCONNECTED;
    return;
  }

  if (_state == MQTT_STATE_CONNECTED && !_mqtt.connected())
  {
    logError("MQTT", "Koneksi ke broker terputus!");
    _state = MQTT_STATE_DISCONNECTED;
  }

  if (_state == MQTT_STATE_DISCONNECTED)
  {
    if (millis() - _lastAttemptAt >= MQTT_RETRY_INTERVAL_MS)
    {
      tryConnect();
    }
  }
}

void MqttManager::loopClient()
{
  _mqtt.loop();
}

bool MqttManager::isConnected()
{
  return _state == MQTT_STATE_CONNECTED;
}

bool MqttManager::publish(const char* topic, const char* payload)
{
  if (_state != MQTT_STATE_CONNECTED)
  {
    logWarn("MQTT", "Tidak terkirim (belum tersambung): %s", payload);
    return false;
  }

  bool ok = _mqtt.publish(topic, payload);
  if (ok)
  {
    logInfo("MQTT", "Terkirim -> %s", payload);
  }
  else
  {
    logError("MQTT", "Publish GAGAL ke topic %s", topic);
  }
  return ok;
}
