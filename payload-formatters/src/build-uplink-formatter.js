const fs = require("fs");

const {
  dataTypes,
  getStructObject,
  decodeStructObject,
  decodeUplink,
} = require("./base-uplink-formatters");

function buildUplinkFormatter(parsedStructs) {
  return (
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
    "\n"
  );
}

module.exports = {
  buildUplinkFormatter,
};
