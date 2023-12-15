#include "include.h"

template <>
meterStruct parseStruct<meterStruct>(DynamicJsonDocument doc, uint16_t counterValue)
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

template <>
tracerStruct parseStruct<tracerStruct>(DynamicJsonDocument doc, uint16_t counterValue)
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

template <>
co2Struct parseStruct<co2Struct>(DynamicJsonDocument doc, uint16_t counterValue)
{
    return (co2Struct){
        doc["S8"]["CarbonDioxide"],
        doc["ANALOG"]["Illuminance"],
        now(),
        counterValue,
    };
}

template <>
coolboxStruct parseStruct<coolboxStruct>(DynamicJsonDocument doc, uint16_t counterValue)
{
    return (coolboxStruct){
        doc["ANALOG"]["Temperature1"],
        doc["ANALOG"]["Temperature2"],
        doc["COUNTER"]["C1"],
        doc["ANALOG"]["Range3"],
        doc["ANALOG"]["Range4"],
        doc["DS18B20"]["Temperature"],
        doc["AM2301"]["Temperature"],
        doc["AM2301"]["Humidity"],
        now(),
        counterValue,
    };
}