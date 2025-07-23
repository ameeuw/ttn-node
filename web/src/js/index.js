import "jsvectormap/dist/css/jsvectormap.css";
import "../css/style.css";
import "@material-design-icons/font/filled.css";
import * as decoder from "../../../payload-formatters/dist/uplink-formatters";

import Alpine from "alpinejs";
import persist from "@alpinejs/persist";
import chart01 from "./components/chart-01";
import chart02 from "./components/chart-02";
import chart03 from "./components/chart-03";
import chart04 from "./components/chart-04";
import mqtt from "mqtt"; // import namespace "mqtt"

Alpine.plugin(persist);
window.Alpine = Alpine;

if (navigator.userAgent.length < 50) {
  const client = mqtt.connect("ws://192.168.4.1:8080");

  client.on("connect", () => {
    client.subscribe("#", (err) => {
      if (!err) {
        client.publish("presence", "Hello mqtt");
      }
    });
  });

  client.on("message", (topic, message) => {
    // message is Buffer
    console.log(topic);
    console.log(message.toString());
  });
} else {
  console.log(
    `Because of the user agent "${navigator.userAgent}" we cannot use the mqtt client`
  );
}

function getChipModel(model) {
  switch (model) {
    case 1:
      return "ESP32";
    case 2:
      return "ESP32-S2";
    case 5:
      return "ESP32-C3";
    case 9:
      return "ESP32-S3";
    case 12:
      return "ESP32-C2";
    case 13:
      return "ESP32-C6";
    case 16:
      return "ESP32-H2";
    case 18:
      return "ESP32-P4";
    case 999:
      return "POSIX/Linux simulator";
    default:
      return "Unknown";
  }
}

function getWakeUpReason(reason) {
  switch (key) {
    case 2:
      return "Wake-up from external signal with RTC_IO";
    case 3:
      return "Wake-up from external signal with RTC_CNTL";
    case 4:
      return "Wake-up caused by a timer";
    case 5:
      return "Wake-up caused by a touchpad";
    case 6:
      return "Wake-up caused by ULP program";
    case 7:
      return "Wake-up caused by GPIO";
    case 8:
      return "Wake-up cause by UART";
    default:
      return "Wake-up reason not identified";
  }
}

function getLmicEvent(event) {
  const LmicEventsEnum = {
    EV_UKNOWN: 0,
    EV_SCAN_TIMEOUT: 1,
    EV_BEACON_FOUND: 2,
    EV_BEACON_MISSED: 3,
    EV_BEACON_TRACKED: 3,
    EV_JOINING: 4,
    EV_JOINED: 5,
    EV_RFU1: 6,
    EV_JOIN_FAILED: 7,
    EV_REJOIN_FAILED: 8,
    EV_TXCOMPLETE: 9,
    EV_LOST_TSYNC: 10,
    EV_RESET: 11,
    EV_RXCOMPLETE: 12,
    EV_LINK_DEAD: 13,
    EV_LINK_ALIVE: 14,
    EV_SCAN_FOUND: 15,
    EV_TXSTART: 16,
    EV_TXCANCELED: 17,
    EV_RXSTART: 18,
    EV_JOIN_TXCOMPLETE: 19,
  };
  switch (key) {
    case LmicEventsEnum.EV_SCAN_TIMEOUT:
      return "EV_SCAN_TIMEOUT";
    case LmicEventsEnum.EV_BEACON_FOUND:
      return "EV_BEACON_FOUND";
    case LmicEventsEnum.EV_BEACON_MISSED:
      return "EV_BEACON_MISSED";
    case LmicEventsEnum.EV_BEACON_TRACKED:
      return "EV_BEACON_TRACKED";
    case LmicEventsEnum.EV_JOINING:
      return "EV_JOINING";
    case LmicEventsEnum.EV_JOINED:
      return "EV_JOINED";
    case LmicEventsEnum.EV_RFU1:
      return "EV_RFU1";
    case LmicEventsEnum.EV_JOIN_FAILED:
      return "EV_JOIN_FAILED";
    case LmicEventsEnum.EV_REJOIN_FAILED:
      return "EV_REJOIN_FAILED";
    case LmicEventsEnum.EV_TXCOMPLETE:
      return "EV_TXCOMPLETE";
    case LmicEventsEnum.EV_LOST_TSYNC:
      return "EV_LOST_TSYNC";
    case LmicEventsEnum.EV_RESET:
      return "EV_RESET";
    case LmicEventsEnum.EV_RXCOMPLETE:
      return "EV_RXCOMPLETE";
    case LmicEventsEnum.EV_LINK_DEAD:
      return "EV_LINK_DEAD";
    case LmicEventsEnum.EV_LINK_ALIVE:
      return "EV_LINK_ALIVE";
    case LmicEventsEnum.EV_SCAN_FOUND:
      return "EV_SCAN_FOUND";
    case LmicEventsEnum.EV_TXSTART:
      return "EV_TXSTART";
    case LmicEventsEnum.EV_TXCANCELED:
      return "EV_TXCANCELED";
    case LmicEventsEnum.EV_RXSTART:
      return "EV_RXSTART";
    case LmicEventsEnum.EV_JOIN_TXCOMPLETE:
      return "EV_JOIN_TXCOMPLETE";
    default:
      return `Unknown event (${event})`;
  }
}

Alpine.store("clientRegistry", {
  rows: {},
  headers: [],
  update(data) {
    this.headers = ["mac", "hostname", "ip", "topic"];
    if (!data.registry.client) return;
    this.rows = Object.keys(data.registry.client).map((key) => {
      return {
        mac: key,
        ...data.registry.client[key],
      };
    });
  },
});

Alpine.store("tasmotaRegistry", {
  rows: {},
  headers: [],
  update(data) {
    this.headers = ["mac", "hostname", "ip", "topic"];
    if (!data.registry.tasmota) return;
    this.rows = Object.keys(data.registry.tasmota).map((key) => {
      return {
        mac: key,
        ...data.registry.tasmota[key],
      };
    });
  },
});

Alpine.store("lastUplinks", {
  rows: {},
  headers: [],
  update(data) {
    this.headers = ["fPort", "length", "bytes"];
    if (!data.lora?.uplink?.last) return;
    this.rows = data.lora.uplink.last.map((item) => {
      try {
        return {
          ...item,
          bytes: JSON.stringify(decoder.decodeUplink(item).data, null, 2),
        };
      } catch (error) {
        console.error(error);
        return {
          ...item,
          bytes: "Error decoding",
        };
      }
    });
  },
});

Alpine.store("taskRegistry", {
  rows: {},
  headers: [],
  update(data) {
    this.headers = ["name", "stack", "priority"];
    this.rows = Object.values(data.tasks).sort((a, b) => b.stack - a.stack);
  },
});

Alpine.store("memoryCards", {
  data: {},
  update(data) {
    this.data = [
      {
        title: "Flash",
        value: `${Math.round(data.system.flash.used / 1000)}k / ${Math.round(
          data.system.flash.size / 1000
        )}k`,
        icon: "storage",
        trend: `${(
          (data.system.flash.used / data.system.flash.size) *
          100
        ).toFixed(2)} %`,
      },
      {
        title: "File System",
        value: `${Math.round(data.system.fs.used / 1000)}k / ${Math.round(
          data.system.fs.size / 1000
        )}k`,
        icon: "sd_card",
        trend: `${((data.system.fs.used / data.system.fs.size) * 100).toFixed(
          2
        )} %`,
      },
      {
        title: "Program",
        value: `${Math.round(data.system.program.used / 1000)}k / ${Math.round(
          data.system.program.size / 1000
        )}k`,
        icon: "terminal",
        trend: `${(
          (data.system.program.used / data.system.program.size) *
          100
        ).toFixed(2)} %`,
      },
      {
        title: "Heap",
        value: `${Math.round(
          (data.system.heap.size - data.system.heap.free) / 1000
        )}k / ${Math.round(data.system.heap.size / 1000)}k`,
        icon: "memory",
        trend: `${(
          ((data.system.heap.size - data.system.heap.free) /
            data.system.heap.size) *
          100
        ).toFixed(2)} %`,
      },
    ];
  },
});

Alpine.store("featureCards", {
  data: {},
  update(data) {
    this.data = [
      {
        title: `Cores: ${data.system.cores}`,
        value: `Model: ${getChipModel(data.system.model)}`,
        icon: "developer_board",
        trend: `${[
          data.system.features.psram ? "PSRAM" : undefined,
          data.system.features.wifi ? "WiFi" : undefined,
          data.system.features.bt ? "BT" : undefined,
          data.system.features.ble ? "BLE" : undefined,
        ]
          .filter((e) => e)
          .join(" | ")}`,
      },
      {
        title: `Wakeup: ${data.system.wakeUpReason} | Reset: ${data.system.resetReason}`,
        value: `Time: ${new Date(data.time * 1000).toISOString()}`,
        icon: "history",
        trend: ``,
      },
      {
        title: `${new Date().toLocaleString()} (${
          (new Date().getTimezoneOffset() * 60) / 3600
        })`,
        value: `${new Date(data.time * 1000).toISOString()}`,
        icon: "update",
        trend: `<a href="#" onclick="setTime()">Set Time</a>`,
      },
    ];
  },
});

Alpine.store("loraCards", {
  data: {},
  update(data) {
    this.data = [
      {
        title: `Joined: ${data.lora.joined} | Up: ${data.lora.up} | Down: ${data.lora.down}`,
        value: `RSSI:\xa0${data.lora.rssi}\xa0dBm`,
        icon: "settings_input_antenna",
        trend: `SNR: ${data.lora.snr} dB`,
      },
      {
        title: `Uplink waiting: ${data.lora.uplink.waiting}`,
        value: `${
          data.lora.uplink.next
            ? '<textarea class="font-mono font-thin text-xs h-40" style="color:black">' +
              JSON.stringify(
                decoder.decodeUplink(data.lora.uplink.next).data,
                null,
                2
              ) +
              "</textarea>"
            : ""
        }`,
        icon: "arrow_upward",
        trend: `Sent: ${data.lora.up}`,
      },
      {
        title: `Downlink waiting: ${data.lora.downlink.waiting}`,
        value: `${
          data.lora.downlink.next ? "Next: " + data.lora.downlink.next : ""
        }`,
        icon: "arrow_downward",
        trend: `Received: ${data.lora.down}`,
      },
    ];
  },
});

const development = true;
const host = development ? "http://192.168.4.1" : "";

Alpine.store("system", {
  data: {},
  update() {
    fetch(host + "/status/lora")
      .then((res) => res.json())
      .then((data) => {
        Alpine.store("loraCards").update(data);
      });

    fetch(host + "/status/registry")
      .then((res) => res.json())
      .then((data) => {
        Alpine.store("clientRegistry").update(data);
        Alpine.store("tasmotaRegistry").update(data);
      });

    fetch(host + "/status/task")
      .then((res) => res.json())
      .then((data) => {
        Alpine.store("taskRegistry").update(data);
      });

    fetch(host + "/status/system")
      .then((res) => res.json())
      .then((data) => {
        Alpine.store("memoryCards").update(data);
        Alpine.store("featureCards").update(data);
      });

    fetch(host + "/lora/lastUplinks")
      .then((res) => res.json())
      .then((data) => {
        Alpine.store("lastUplinks").update(data);
      });
  },
});
Alpine.store("system").update();

setInterval(() => {
  Alpine.store("system").update();
}, 5000);

Alpine.start();

window.setTime = () => {
  var d = new Date();
  var timestamp = Math.round(d.getTime() / 1000);
  var offset = d.getTimezoneOffset() * 60;
  console.log(`Timestamp: ${timestamp}`);
  console.log(`Timezone offset: ${offset}`);
  console.log(`Sending time: timestamp + offset = ${timestamp - offset}`);

  let command = {
    cmd: "CMD_SET_TIME",
    param: timestamp - offset + 1,
  };
  fetch(`${host}/command?cmd=${command.cmd}&param=${command.param}`);
};

window.decodeThing = () => {
  console.log("blubb");
  const input = {
    fPort: 15,
    bytes: [
      0x15, 0xae, 0x47, 0xe1, 0x7a, 0xb4, 0x47, 0x40, 0x15, 0xae, 0x47, 0xe1,
      0x7a, 0x14, 0x21, 0x40, 0x33, 0x33, 0x33, 0x33, 0x33, 0x1f, 0x80, 0x40,
      0xf6, 0x28, 0x5c, 0x8f, 0xc2, 0xf5, 0xf0, 0x3f, 0x5f, 0xcd, 0x7c, 0x65,
      0x39, 0x05, 0xfd, 0x3f,
    ],
  };
  const decodedPayload = decoder.decodeUplink(input);
  return decodedPayload;
};

// Document Loaded
document.addEventListener("DOMContentLoaded", () => {
  chart01();
  chart02();
  chart03();
  chart04();
});
