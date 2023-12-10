#ifndef PARSERS_H_
#define PARSERS_H_

#include "include.h"

meterStruct parseMeterStruct(DynamicJsonDocument doc, uint16_t counterValue);
tracerStruct parseTracerStruct(DynamicJsonDocument doc, uint16_t counterValue);
co2Struct parseCo2Struct(DynamicJsonDocument doc, uint16_t counterValue);

#endif
