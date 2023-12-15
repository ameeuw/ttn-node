const { decodeUplink } = require("./base-uplink-formatters");
const parsedStructs = require("./parsed-structs.json");

function assertObjectEquality(actual, expected) {
  if (JSON.stringify(actual) !== JSON.stringify(expected)) {
    console.log("Expected:", expected);
    console.log("Actual:", actual);
    throw new Error("Assertion failed");
  }
}

const payloads = {
  gps: {
    payload: {
      fPort: 15,
      bytes: [
        0x15, 0xae, 0x47, 0xe1, 0x7a, 0xb4, 0x47, 0x40, 0x15, 0xae, 0x47, 0xe1,
        0x7a, 0x14, 0x21, 0x40, 0x33, 0x33, 0x33, 0x33, 0x33, 0x1f, 0x80, 0x40,
        0xf6, 0x28, 0x5c, 0x8f, 0xc2, 0xf5, 0xf0, 0x3f, 0x5f, 0xcd, 0x7c, 0x65,
        0x39, 0x05, 0xfd, 0x3f,
      ],
    },
    parsed: {
      data: {
        latitude: 47.410000000000004,
        longitude: 8.540000000000001,
        altitude: 515.9,
        speed: 1.06,
        t: 1702677855,
        counter: 1337,
        fport: 253,
      },
      warnings: [],
    },
  },
};

for (const [name, { payload, parsed }] of Object.entries(payloads)) {
  const actual = decodeUplink(payload, parsedStructs);
  console.log("- testing", name);
  assertObjectEquality(actual, parsed);
}
