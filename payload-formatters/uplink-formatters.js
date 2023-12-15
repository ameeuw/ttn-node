/*******************************************************************************
 *
 *  File:         lmic-node-uplink-formatters.js
 *
 *  Function:     TBeam LMIC Node Uplink Formatters
 *
 *  Author:       Arne Meeuw
 *
 *  Description:  These function(s) are for use with The Things Network V3.
 *
 ******************************************************************************/

// Parsed structs
const parsedStructs = [
  {
    id: 11,
    name: "telemetry",
    structString:
      "struct telemetry{float temp;float hum;float voltage;float current;float power;uint16_t counter;uint32_t t;static constexpr uint8_t fport = 11;};",
  },
  {
    id: 12,
    name: "tracerStruct",
    structString:
      "struct tracerStruct{float batteryTemperature;float batterySoc;float batteryVoltage;float batteryCurrent;float pvVoltage;float pvCurrent;float pvPower;float loadVoltage;float loadCurrent;float loadPower;float consumptionDay;float consumptionSum;float productionSum;float batteryMaxVoltage;int32_t t;uint16_t counter;static constexpr uint8_t fport = 12;};",
  },
  {
    id: 13,
    name: "meterStruct",
    structString:
      "struct meterStruct{float batteryVoltage;float loadPower;float consumptionSum;int32_t t;uint16_t counter;static constexpr uint8_t fport = 13;};",
  },
  {
    id: 14,
    name: "co2Struct",
    structString:
      "struct co2Struct{uint16_t co2;uint16_t illuminance;int32_t t;uint16_t counter;static constexpr uint8_t fport = 14;};",
  },
  {
    id: 15,
    name: "gpsStruct",
    structString:
      "struct gpsStruct{double latitude;double longitude;double altitude;double speed;int32_t t;uint16_t counter;static constexpr uint8_t fport = 15;};",
  },
  {
    id: 16,
    name: "coolboxStruct",
    structString:
      "struct coolboxStruct{float inflowTemperature;float outflowTemperature;uint16_t flowCounter;uint16_t tec1Current;uint16_t tec2Current;float waterTemperature;float airTemperature;float humidity;int32_t t;uint16_t counter;static constexpr uint8_t fport = 16;};",
  },
];

// dataTypes and their DataView getters
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

// Parses a C struct as an object with name and members with names and types
function getStructObject(structString) {
  const regex =
    /struct\s+(\w+)\s*\{((?:\s*(?:static constexpr)?\s*\w+\s+\w+\s*(?:=\s*\d+)?\s*;\s*)+)\};?/;
  const match = regex.exec(structString);
  if (match) {
    return {
      name: match[1],
      members: match[2]
        .trim()
        .split(/\s*;\s*/)
        .filter((member) => member.length > 0) // Filter out empty strings
        .map((member) => {
          const parts = member.trim().split(/\s+/);
          let value;
          if (parts.includes("=")) {
            value = parseInt(parts.pop());
            parts.pop(); // remove the '=' symbol
          }
          const name = parts.pop();
          const type = parts
            .filter((part) => part !== "static" && part !== "constexpr")
            .join(" ");
          return { type, name, value };
        }),
    };
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
  } else {
    const structObject = parsedStructs.find(
      (struct) => struct.id === input.fPort
    );
    if (structObject) {
      data = {
        ...decodeStructObject(
          getStructObject(structObject.structString),
          input.bytes
        ),
      };
    } else {
      warnings.push("Unsupported fPort");
    }
  }
  return {
    data: data,
    warnings: warnings,
  };
}

console.log(decodeUplink.toString());

module.exports = decodeUplink;
