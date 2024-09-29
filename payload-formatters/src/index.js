const fs = require("fs");

const { parseStructsFromFile } = require("./parse-typedef");
const { buildUplinkFormatter } = require("./build-uplink-formatter");
const { join } = require("path");

const filePath = "../include/typedef.h";

const parsedStructs = parseStructsFromFile(filePath);
const outString = buildUplinkFormatter(parsedStructs);

const outDir = "dist";

fs.writeFileSync(join(outDir, "uplink-formatters.js"), outString);
