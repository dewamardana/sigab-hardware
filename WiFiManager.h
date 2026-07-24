/*
  ============================================================
  WiFiManager.h
  Mengelola koneksi WiFi secara NON-BLOCKING.

  Kenapa non-blocking?
  Kalau WiFi putus lalu kita pakai `while (WiFi.status() != WL_CONNECTED) {}`,
  seluruh program (termasuk pembacaan sensor & keselamatan sistem
  monitoring sungai) ikut berhenti total selama itu, dan ESP32 bisa
  ter-reset sendiri oleh watchdog timer. Sebagai gantinya, dipakai
  state machine: sistem tetap jalan (sensor tetap dibaca, data tetap
  dicatat) sambil di background terus mencoba reconnect setiap
  WIFI_RETRY_INTERVAL_MS.
  ============================================================
*/
#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "Config.h"

enum WiFiState {
  WIFI_STATE_DISCONNECTED,
  WIFI_STATE_CONNECTING,
  WIFI_STATE_CONNECTED
};

class WiFiManager {
  public:
    void begin();
    void update();                 // panggil tiap loop(), non-blocking
    bool isConnected();
    WiFiState getState();

  private:
    WiFiState _state = WIFI_STATE_DISCONNECTED;
    unsigned long _connectStartedAt = 0;
    unsigned long _lastAttemptAt = 0;
    bool _wasConnected = false;
    void startConnect();
};

#endif // WIFI_MANAGER_H
