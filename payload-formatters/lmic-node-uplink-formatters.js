/*******************************************************************************
 *
 *  File:         lmic-node-uplink-formatters.js
 *
 *  Function:     LMIC-node uplink payload formatter JavaScript function(s).
 *
 *  Author:       Leonel Lopes Parente
 *
 *  Description:  These function(s) are for use with The Things Network V3.
 *
 ******************************************************************************/

const dataTypes = {
  float: { length: 32, getter: "getFloat32" },
  double: { length: 64, getter: "getFloat64" },
  int8_t: { length: 8, getter: "getInt8" },
  int16_t: { length: 16, getter: "getInt16" },
  int32_t: { length: 32, getter: "getInt32" },
  int64_t: { length: 64, getter: "getInt64" },
  uint8_t: { length: 8, getter: "getUint8" },
  uint16_t: { length: 16, getter: "getUint16" },
  uint32_t: { length: 32, getter: "getUint32" },
  uint64_t: { length: 64, getter: "getUint64" },
};

const telemetryStruct = `
  struct telemetry
  {
    float temp;
    float hum;
    float voltage;
    float current;
    float power;
    uint16_t counter;
    uint32_t t;
  };`;

const tracerStruct = `
struct tracer
{
    float batteryTemperature;
    float batterySoc;
    float batteryVoltage;
    float batteryCurrent;
    float pvVoltage;
    float pvCurrent;
    float pvPower;
    float loadVoltage;
    float loadCurrent;
    float loadPower;
    float consumptionDay;
    float consumptionSum;
    float productionSum;
    float batteryMaxVoltage;
    uint16_t counter;
    uint32_t t;
};`;

const meterStruct = `
struct meter
{
    float batteryVoltage;
    float loadPower;
    float consumptionSum;
    uint16_t counter;
    uint32_t t;
};`;

const co2Struct = `
struct co2
{
    uint16_t co2;
    uint16_t illuminance;
    uint32_t t;
    uint16_t counter;
};`;

// Parses a C struct as an object with name and members with names and types
function getStructObject(structString) {
  const regex = /struct\s+(\w+)\s*\{((?:\s*\w+\s+\w+\s*;\s*)+)\};?/;
  const match = regex.exec(structString);
  if (match) {
    return (structObj = {
      name: match[1],
      members: match[2]
        .trim()
        .split(/\s*;\s/)
        .map((member) => {
          const [type, name] = member.trim().split(/\s+/);
          return { type, name };
        }),
    });
  } else {
    console.log("No struct found.");
    return {};
  }
}

// Decodes data from an array of bytes according to the specifications of a structObject
function decodeStructObject(structObject, bytes) {
  let offset = 0;
  return structObject.members.reduce((acc, { type, name }) => {
    const { length, getter } = dataTypes[type];
    // Built dataView on byteArray
    const byteArray = new Uint8Array(bytes);
    const view = new DataView(byteArray.buffer);
    // Read the binary data as an according type
    acc[name] = view[getter](offset, true);
    offset += length / 8;
    return acc;
  }, {});
}

function decodeUplink(input) {
  var data = {};
  var warnings = [];

  if (input.fPort == 10) {
    data.counter = (input.bytes[0] << 8) + input.bytes[1];
  } else if (input.fPort == 11) {
    data = {
      ...decodeStructObject(getStructObject(telemetryStruct), input.bytes),
    };
  } else if (input.fPort == 12) {
    data = {
      ...decodeStructObject(getStructObject(tracerStruct), input.bytes),
    };
  } else if (input.fPort == 13) {
    data = {
      ...decodeStructObject(getStructObject(meterStruct), input.bytes),
    };
  } else if (input.fPort == 14) {
    data = {
      ...decodeStructObject(getStructObject(co2Struct), input.bytes),
    };
  } else {
    warnings.push("Unsupported fPort");
  }
  return {
    data: data,
    warnings: warnings,
  };
}
