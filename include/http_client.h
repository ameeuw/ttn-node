#ifndef HTTP_CLIENT_H_
#define HTTP_CLIENT_H_

#include "include.h"

meter parseMeterStruct(JsonObject doc, uint16_t counterValue);
tracer parseTracerStruct(JsonObject doc, uint16_t counterValue);
co2 parseCo2Struct(JsonObject doc, uint16_t counterValue);

#endif
