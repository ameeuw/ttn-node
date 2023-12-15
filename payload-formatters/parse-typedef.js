const fs = require("fs");

const filePath = "../include/typedef.h";
const outputFile = "./parsed-structs.json";
const structIdentifier = "fport";

function parseStructsFromFile(filePath) {
  const fileContent = fs.readFileSync(filePath, "utf8");

  // Split the file content into lines
  const lines = fileContent.split("\n");

  // Initialize an empty array to hold the structs
  const structs = [];

  // Initialize an empty string to hold the current struct
  let structString = "";
  let name, id;

  // Iterate over the lines
  for (const line of lines) {
    // If the line is the start of a struct, start a new structString
    if (line.trim().startsWith("struct")) {
      // Get the struct name from the line using a regex match and capture group
      const regex = /struct\s+(\w+)/;
      const match = line.match(regex);
      if (match) {
        name = match[1];
      }
      structString = line;
    }
    // If the line is the end of a struct, add the structString to the structs array
    else if (line.trim().startsWith("};")) {
      structString += line;
      structs.push({
        id,
        name,
        structString,
        // structObject: getStructObject(structString),
      });
      structString = "";
    }
    // If the line is part of a struct, add it to the structString
    else if (structString.length > 0) {
      // Get the struct id from the line using a regex match and capture group
      const regex = new RegExp(`${structIdentifier} = (\\d+);`);
      const match = line.match(regex);
      if (match) {
        // Id is assumed to be an integer
        id = parseInt(match[1]);
      }
      structString += line.trim();
    }
  }

  return structs;
}

const payloadParsers = parseStructsFromFile("../include/typedef.h");
fs.writeFileSync(outputFile, JSON.stringify(payloadParsers, null, 2));
