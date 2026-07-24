#include "WiFiManager.h"
#include "Logger.h"

void WiFiManager::begin()
{
  WiFi.mode(WIFI_STA);
  startConnect();
}

void WiFiManager::startConnect()
{
  logInfo("WIFI", "Mencoba konek ke SSID '%s'...", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  _state = WIFI_STATE_CONNECTING;
  _connectStartedAt = millis();
  _lastAttemptAt = millis();
}

void WiFiManager::update()
{
  switch (_state)
  {
    case WIFI_STATE_DISCONNECTED:
      // Coba konek lagi tiap WIFI_RETRY_INTERVAL_MS, tanpa nge-block loop()
      if (millis() - _lastAttemptAt >= WIFI_RETRY_INTERVAL_MS)
      {
        startConnect();
      }
      break;

    case WIFI_STATE_CONNECTING:
      if (WiFi.status() == WL_CONNECTED)
      {
        _state = WIFI_STATE_CONNECTED;
        _wasConnected = true;
        logInfo("WIFI", "Terhubung. IP: %s", WiFi.localIP().toString().c_str());
      }
      else if (millis() - _connectStartedAt >= WIFI_CONNECT_TIMEOUT_MS)
      {
        logWarn("WIFI", "Timeout konek, akan dicoba lagi dalam %lu ms", WIFI_RETRY_INTERVAL_MS);
        _state = WIFI_STATE_DISCONNECTED;
        _lastAttemptAt = millis();
      }
      break;

    case WIFI_STATE_CONNECTED:
      if (WiFi.status() != WL_CONNECTED)
      {
        logError("WIFI", "Koneksi terputus! Masuk mode reconnect.");
        _state = WIFI_STATE_DISCONNECTED;
        _lastAttemptAt = millis();
      }
      break;
  }
}

bool WiFiManager::isConnected()
{
  return _state == WIFI_STATE_CONNECTED;
}

WiFiState WiFiManager::getState()
{
  return _state;
}
