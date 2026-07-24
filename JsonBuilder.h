/*
  ============================================================
  JsonBuilder.h - Bangun payload JSON dari RiverData
  ============================================================
*/
#ifndef JSON_BUILDER_H
#define JSON_BUILDER_H

#include <Arduino.h>
#include "RiverData.h"

String buatJSON(const RiverData &data);

#endif // JSON_BUILDER_H
