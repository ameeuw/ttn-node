#ifndef HTTP_CLIENT_H_
#define HTTP_CLIENT_H_

#include "include.h"

meterStruct parseMeterStruct(JsonObject doc, uint16_t counterValue);
tracerStruct parseTracerStruct(JsonObject doc, uint16_t counterValue);
co2Struct parseCo2Struct(DynamicJsonDocument doc, uint16_t counterValue);

#endif
