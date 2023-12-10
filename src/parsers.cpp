#include "include.h"

meterStruct parseMeterStruct(DynamicJsonDocument doc, uint16_t counterValue)
{
    meterStruct meterPayload = {
        ((float)doc["ANALOG"]["Range"]) / 1000,
        doc["meter"]["power"],
        doc["meter"]["consumption"],
        now(),
        counterValue,
    };
    return meterPayload;
}

tracerStruct parseTracerStruct(DynamicJsonDocument doc, uint16_t counterValue)
{
    tracerStruct tracerPayload = {
        doc["TRACER"]["batteryTemperature"],
        doc["TRACER"]["batterySoc"],
        doc["TRACER"]["batteryVoltage"],
        doc["TRACER"]["batteryCurrent"],
        doc["TRACER"]["pvVoltage"],
        doc["TRACER"]["pvCurrent"],
        doc["TRACER"]["pvPower"],
        doc["TRACER"]["loadVoltage"],
        doc["TRACER"]["loadCurrent"],
        doc["TRACER"]["loadPower"],
        doc["TRACER"]["consumptionDay"],
        doc["TRACER"]["consumptionSum"],
        doc["TRACER"]["productionSum"],
        doc["TRACER"]["batteryMaxVoltage"],
        now(),
        counterValue,
    };
    return tracerPayload;
}

co2Struct parseCo2Struct(DynamicJsonDocument doc, uint16_t counterValue)
{
    return (co2Struct){
        doc["S8"]["CarbonDioxide"],
        doc["ANALOG"]["Illuminance"],
        now(),
        counterValue,
    };
}