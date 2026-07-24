/*
  ============================================================
  Logger.h
  Fungsi debug print terpusat dengan level (ERROR/WARN/INFO/DEBUG)
  dan timestamp (millis), supaya semua modul memakai format
  output yang konsisten dan mudah dicari saat troubleshooting.

  Cara pakai di modul lain:
    logError("WIFI", "Gagal konek setelah timeout");
    logWarn("TFLUNA", "Checksum gagal, retry ke-%d", retryCount);
    logInfo("SYSTEM", "Semua sensor OK");
    logDebug("RAIN", "Tip masuk, total=%lu", tipCount);
  ============================================================
*/
#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "Config.h"

inline void logPrint(const char* level, const char* tag, const char* fmt, va_list args)
{
  char buf[160];
  vsnprintf(buf, sizeof(buf), fmt, args);

  Serial.print('[');
  Serial.print(millis());
  Serial.print(" ms][");
  Serial.print(level);
  Serial.print("][");
  Serial.print(tag);
  Serial.print("] ");
  Serial.println(buf);
}

inline void logError(const char* tag, const char* fmt, ...)
{
#if LOG_LEVEL >= 1
  va_list args;
  va_start(args, fmt);
  logPrint("ERROR", tag, fmt, args);
  va_end(args);
#endif
}

inline void logWarn(const char* tag, const char* fmt, ...)
{
#if LOG_LEVEL >= 2
  va_list args;
  va_start(args, fmt);
  logPrint("WARN ", tag, fmt, args);
  va_end(args);
#endif
}

inline void logInfo(const char* tag, const char* fmt, ...)
{
#if LOG_LEVEL >= 3
  va_list args;
  va_start(args, fmt);
  logPrint("INFO ", tag, fmt, args);
  va_end(args);
#endif
}

inline void logDebug(const char* tag, const char* fmt, ...)
{
#if LOG_LEVEL >= 4
  va_list args;
  va_start(args, fmt);
  logPrint("DEBUG", tag, fmt, args);
  va_end(args);
#endif
}

#endif // LOGGER_H
