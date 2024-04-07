import "jsvectormap/dist/css/jsvectormap.css";
import "../css/style.css";
import "@material-design-icons/font/filled.css";

import Alpine from "alpinejs";
import persist from "@alpinejs/persist";
import chart01 from "./components/chart-01";
import chart02 from "./components/chart-02";
import chart03 from "./components/chart-03";
import chart04 from "./components/chart-04";

Alpine.plugin(persist);
window.Alpine = Alpine;

function printChipModel(model) {
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

function printWakeUpReason(reason) {
  switch (key) {
    case 2:
      wakeupReasonString = "Wake-up from external signal with RTC_IO";
      break;
    case 3:
      wakeupReasonString = "Wake-up from external signal with RTC_CNTL";
      break;
    case 4:
      wakeupReasonString = "Wake-up caused by a timer";
      break;
    case 5:
      wakeupReasonString = "Wake-up caused by a touchpad";
      break;
    case 6:
      wakeupReasonString = "Wake-up caused by ULP program";
      break;
    case 7:
      wakeupReasonString = "Wake-up caused by GPIO";
      break;
    case 8:
      wakeupReasonString = "Wake-up cause by UART";
      break;

    default:
      wakeupReasonString = "Wake-up reason not identified";
      break;
  }
  return wakeupReasonString;
}

Alpine.store("clientRegistry", {
  rows: {},
  headers: [],
  update(data) {
    this.headers = ["mac", "hostname", "ip", "topic"];
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

Alpine.store("taskRegistry", {
  rows: {},
  headers: [],
  update(data) {
    this.headers = ["name", "stack", "priority"];
    this.rows = Object.values(data.tasks);
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
        icon: "memory",
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
        icon: "memory",
        trend: `${((data.system.fs.used / data.system.fs.size) * 100).toFixed(
          2
        )} %`,
      },
      {
        title: "Program",
        value: `${Math.round(data.system.program.used / 1000)}k / ${Math.round(
          data.system.program.size / 1000
        )}k`,
        icon: "memory",
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
        value: `Model: ${printChipModel(data.system.model)}`,
        icon: "memory",
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
        title: `Up: ${data.lora.up} | Down: ${data.lora.down}`,
        value: `RSSI: ${data.lora.rssi} dBm`,
        icon: "memory",
        trend: `SNR: ${data.lora.snr} dB`,
      },
      {
        title: `Wakeup: ${data.system.wakeUpReason} | Reset: ${data.system.resetReason}`,
        value: `Time: ${data.time}`,
        icon: "memory",
        trend: ``,
      },
    ];
  },
});

Alpine.store("system", {
  data: {},
  update() {
    fetch("/status")
      .then((res) => res.json())
      .then((data) => {
        this.data = data;
        Alpine.store("clientRegistry").update(data);
        Alpine.store("tasmotaRegistry").update(data);
        Alpine.store("taskRegistry").update(data);
        Alpine.store("memoryCards").update(data);
        Alpine.store("featureCards").update(data);
      });
  },
});
Alpine.store("system").update();

setInterval(() => {
  Alpine.store("system").update();
}, 5000);

Alpine.start();

// Document Loaded
document.addEventListener("DOMContentLoaded", () => {
  chart01();
  chart02();
  chart03();
  chart04();
});
