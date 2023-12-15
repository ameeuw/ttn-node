const fs = require("fs");

const {
  dataTypes,
  getStructObject,
  decodeStructObject,
  decodeUplink,
} = require("./base-uplink-formatters");
const parsedStructs = require("./parsed-structs.json");

const outString =
  "const dataTypes = " +
  JSON.stringify(dataTypes, null, 2) +
  "\n" +
  "const structs = " +
  JSON.stringify(parsedStructs, null, 2) +
  "\n" +
  getStructObject.toString() +
  "\n" +
  decodeStructObject.toString() +
  "\n" +
  decodeUplink.toString() +
  "\n";

fs.writeFileSync("uplink-formatters.js", outString);
