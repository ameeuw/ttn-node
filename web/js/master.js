const START_ADDRESS = 0x0;
const CLOCKGROUP_WIDTH = 210;
const CLOCKGROUP_HEIGHT = 210;

var numerals = [
  [ // zero
    [3, 30],
    [0, 30],
    [0, 15],

    [0, 45],
    [0, 30],
    [6, 45]
  ],
  [ // one
    [7, 35],
    [7, 35],
    [7, 35],

    [0, 0],
    [0, 30],
    [6, 30]
  ],
  [ // two
    [3, 15],
    [3, 30],
    [0, 15],

    [9, 45],
    [0, 45],
    [6, 45]
  ],
  [ // three
    [3, 15],
    [3, 15],
    [3, 15],

    [0, 45],
    [0, 45],
    [6, 45]
  ],
  [ // four
    [0, 30],
    [0, 15],
    [7, 35],

    [0, 0],
    [0, 15],
    [6, 30]
  ],
  [ // five
    [3, 30],
    [0, 15],
    [3, 15],

    [0, 45],
    [9, 30],
    [9, 45]
  ],
  [ // six
    [3, 30],
    [0, 30],
    [0, 15],

    [0, 45],
    [9, 30],
    [9, 45]
  ],
  [ // seven
    [3, 45],
    [3, 15],
    [7, 35],

    [0, 0],
    [0, 30],
    [6, 45]
  ],
  [ // eight
    [3, 30],
    [0, 20],
    [0, 15],

    [0, 45],
    [0, 40],
    [6, 45]
  ],
  [ // nine
    [3, 30],
    [0, 15],
    [3, 15],

    [0, 45],
    [0, 30],
    [6, 45]
  ]
];

function ClockMaster() {
  this.generator = {};
  this.generator.x = 0;
  this.messageBus = new MessageBus();
  this.clockGroups = [];
  this.clockGroups.push( new ClockGroup(clock, 0, CLOCKGROUP_WIDTH, CLOCKGROUP_HEIGHT) );
  this.clockGroups.push( new ClockGroup(clock, 1, CLOCKGROUP_WIDTH, CLOCKGROUP_HEIGHT) );
  this.clockGroups.push( new ClockGroup(clock, 2, CLOCKGROUP_WIDTH, CLOCKGROUP_HEIGHT) );
  this.clockGroups.push( new ClockGroup(clock, 3, CLOCKGROUP_WIDTH, CLOCKGROUP_HEIGHT) );
  this.clockGroups.forEach((clockGroup)=>{
    clockGroup.clocks.forEach((clock)=>{
      this.messageBus.addListener(
        clock.address,
        clock
      );
    });
  });
}

ClockMaster.prototype.muster = function() {
  this.generator.x++;
  let x = this.generator.x;
  let phi = Math.PI * (x / 10);
  let y = (Math.sin(phi) / 2) + 0.5;
  y = Math.round(y*1000)/1000;
  // console.log(`${x} / ${y}`);
  // console.log(phi);
  // console.log(`Math.sin(x) = ${Math.sin(x)}`);
  for (let clock = 0; clock < (CLOCKS_PER_COLUMN*COLUMNS_PER_GROUP*4); clock++) {
    this.setClock(clock, y*2, -y*10);
  }
}

ClockMaster.prototype.update = function() {
  var d = new Date();
  let setDigits = [0,0,0,0];
  if (d.getHours() < 10) {
    setDigits[0] = 0;
    setDigits[1] = d.getHours();
  } else {
    setDigits[0] = Math.floor(d.getHours()/10);
    setDigits[1] = d.getHours()%10;
  }

  if (d.getMinutes() < 10) {
    setDigits[2] = 0;
    setDigits[3] = d.getMinutes();
  } else {
    setDigits[2] = Math.floor(d.getMinutes()/10);
    setDigits[3] = d.getMinutes()%10;
  }

  let NUM_CLOCKGROUPS = 4;

  for (var clockGroup = 0; clockGroup < NUM_CLOCKGROUPS; clockGroup++) {
    this.setClockgroup(clockGroup, setDigits[clockGroup]);
  }
}

ClockMaster.prototype.setClockgroup = function(clockGroup, numeral) {
  for (var clock = 0; clock < (CLOCKS_PER_COLUMN*COLUMNS_PER_GROUP); clock++) {
    let hour = numerals[numeral][clock][0];
    let minute = numerals[numeral][clock][1];
    let address = START_ADDRESS + clockGroup * (CLOCKS_PER_COLUMN*COLUMNS_PER_GROUP) + clock;
    // console.log(`Sending ${hour}:${minute} to #${address}`);
    this.setClock(address, hour, minute);
  }
}

ClockMaster.prototype.setClock = function(address, hour, minute) {
  this.messageBus.send(
    address,
    hour,
    minute
  );
}
