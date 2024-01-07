function Client() {
  this.rtcTime = undefined;
}

Client.prototype.calibrate = function() {
  let command = {
    "cmd": "CMD_CAL"
  }
  this.sendCommand(command);
}

Client.prototype.setTime = function() {
  var d = new Date();
  var timestamp = Math.round(d.getTime()/1000);
  var offset = d.getTimezoneOffset() * 60;
  console.log(`Timestamp: ${timestamp}`);
  console.log(`Timezone offset: ${offset}`);
  console.log(`Sending time: timestamp + offset = ${timestamp-offset}`);

  let command = {
    "cmd": "CMD_SET_TIME",
    "param": timestamp - offset + 1
  }
  this.sendCommand(command);
}

Client.prototype.getTime = function() {
  let command = {
    "cmd": "CMD_GET_TIME"
  };
  let that = this;
  this.sendCommand(command, function(responseText) {
    let timestamp = parseInt(responseText);
    var d = new Date();
    var offset = d.getTimezoneOffset() * 60;
    console.log(`Timestamp: ${timestamp}`);
    console.log(`Timezone offset: ${offset}`);
    that.rtcTime = (timestamp + offset) * 1000;
    // d = new Date(client.rtcTime);
    // alert(d.toLocaleTimeString("de-DE"));
    console.log(`Got RTC timestamp: ${that.rtcTime}`);
  });
}

Client.prototype.random = function() {
  let command = {
    "cmd": "CMD_RANDOM"
  }
  this.sendCommand(command);
}

Client.prototype.set = function() {
  let command = {
    "cmd": "CMD_SET"
  }
  this.sendCommand(command);
}

Client.prototype.sendCommand = function(command = {
  "cmd": "CMD_SET_TIME",
  "param": Math.round(Date.now()/1000)
}, callback) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (xhttp.readyState === 4 && xhttp.status === 200) {
      console.log("responseText:");
      console.log(this.responseText);
      if (callback) {
        callback(this.responseText);
      }
    }
  };

  let commandString = `command?cmd=${command.cmd}&param=${command.param}`;
  console.log(commandString);

  xhttp.open(
    "GET",
    commandString,
    true
  );
  xhttp.send();
}
