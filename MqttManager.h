/*
  ============================================================
  MqttManager.h
  Menggantikan pola:
    while (!mqtt.connected()) { ... delay(5000); }
  yang membuat seluruh loop() berhenti total tiap kali broker
  tidak terjangkau. Di sini dipakai state machine: sistem tetap
  membaca sensor & memproses input Serial selagi MqttManager
  mencoba connect ulang di background setiap
  MQTT_RETRY_INTERVAL_MS.

  Catatan jujur: PubSubClient adalah library yang secara
  internal sinkron - satu panggilan mqtt.connect() bisa
  menahan program sesaat (biasanya < 1-2 detik) kalau broker
  benar-benar tidak terjangkau. Yang dihilangkan di sini adalah
  DELAY TAMBAHAN 5 detik yang menumpuk tiap retry serta
  ketergantungan pada WiFi reconnect di dalam while yang sama.
  ============================================================
*/
#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <WiFi.h>
#include <PubSubClient.h>
#include "Config.h"

enum MqttState {
  MQTT_STATE_DISCONNECTED,
  MQTT_STATE_CONNECTED
};

class MqttManager {
  public:
    void begin();
    void update(bool wifiConnected);   // panggil tiap loop(), non-blocking
    bool isConnected();
    bool publish(const char* topic, const char* payload);
    void loopClient();                 // panggil mqtt.loop() tiap iterasi loop()

  private:
    WiFiClient   _wifiClient;
    PubSubClient _mqtt{_wifiClient};
    MqttState _state = MQTT_STATE_DISCONNECTED;
    unsigned long _lastAttemptAt = 0;

    void tryConnect();
};

#endif // MQTT_MANAGER_H
