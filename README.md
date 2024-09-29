### TTGO T-BEAM LMIC-Node

This code holds the functionality to keep a collection of nodes based on Tasmota working without internet.
The uplink and downlink is realised via LoRaWAN (TheThingsNetwork TTN), local connectivity is handled by 
PicoMQTT and time synchronisation is achieved via GPS.

The process is divided into multiple tasks which either act on incoming events or work off queues.


The LoRaWAN code is heavily based on the LMIC-Node project which seems to be the one-fits-all-solution to anything LoRaWAN.
It is reduced and extended to be processed by FreeRTOS and adds queue-workers for both sides.

### Tested Hardware
- Lopy v1
  - real-time clock
  - SD-card
- TTGO Lora32 
  - OLED display
  - SD-card
- TTGO T-Beam V1.2 
  - GPS
  - display
  - Power management IC

### Tasks
- HandleUplink
- HandleDownlink
- LMIC
- Status
- Mqtt
  
### Queues
- UplinkQueue
- DownlinkQueue

### Payload structs
- Telemetry
- Tracer
- Meter
- CO2
- GPS
- Coolbox

### PicoMQTT
### Tasmota Registry

### Registry functionality
Available nodes post on `tasmota/discovery/<MAC>/config` with payload:

```JSON
{"ip":"192.168.2.196","dn":"Tasmota","fn":["Tasmota",null,null,null,null,null,null,null],"hn":"tasmota-80FF6D-8045","mac":"E0980680FF6D","md":"Generic","ty":0,"if":0,"ofln":"Offline","onln":"Online","state":["OFF","ON","TOGGLE","HOLD"],"sw":"13.3.0.1","t":"tasmota_80FF6D","ft":"%prefix%/%topic%/","tp":["cmnd","stat","tele"],"rl":[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],"swc":[-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1],"swn":[null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null],"btn":[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],"so":{"4":0,"11":0,"13":0,"17":0,"20":0,"30":0,"68":0,"73":0,"82":0,"114":0,"117":0},"lk":0,"lt_st":0,"bat":0,"dslp":0,"sho":[],"sht":[],"ver":1}
```

### Todos
- [ ] Set UPLINK frequency via rule or berry
  - rule1 ON Time#Minute|15 DO status 8 ENDON
  - rule1 1
- [x] Keep registry of discovered tasmota nodes
- [x] Keep registry of connected wifi clients
- [ ] Scan for known sensors and anticipate payloads
- [x] Unify struct parser as template for payload structs
- [x] Automatically export payload formatter using script
- [x] Map structs to fport
- [x] Sync time from GPS
- [x] Update time on list of nodes via MQTT
- [ ] Set teleperiod on registered devices (might be unnecessary)
- [x] Move to tele/+/UPLINK topic to uncouple uplinks from tele messages (probably ´SENSOR8` topic)
- [ ] Unqueue double messages
- [ ] Add prioritisation to linkMessages
- [ ] Check memory allocation and freeing
- [ ] Add back DISTANCE MOVED
- [x] Forward Downlink messages to MQTT
- [ ] Confirm sending of downlink messages via uplink message (check-byte on mirrored fport?)
- [x] Change key management to be equivalent to ttn mapper style
- [ ] Forward LMIC events via MQTT
- [ ] Forward LMIC events to main application
- [ ] Reboot or reconnect on lost LMIC connection
- [ ] Retry failed scheduleUplink calls
- [ ] Status Website served on captive portal (copy functionality from ClockClock)
  - [ ] Last GPS coordinate (maybe even on map?)
  - [x] Current time
  - [x] Links to connected clients with host name, ip address
  - [x] Status of Task heaps
  - [x] System Information and Status: chip, heap, time, clock
  - [x] LoRa Status: joined, snr, rssi
  - [x] LoRa queue: uplink, downlink, next & last messages
  - [x] Payload parsing from TTN parsers
  - [ ] Split status json messages to different endpoints (/system, /lora, /registry, /tasks)
- [x] OTA functionality (https://randomnerdtutorials.com/esp32-ota-over-the-air-vs-code/)