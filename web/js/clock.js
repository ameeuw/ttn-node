function MessageBus() {
  this.listeners = {};
}

MessageBus.prototype.addListener = function(address, object) {
  if (!this.listeners[address]) {
    // console.log(`Added listener at address ${address}`);
    this.listeners[address] = object;
  }
}

MessageBus.prototype.send = function(address, hour, minute) {
  if (this.listeners[address]) {
    this.listeners[address].setTime(hour, minute);
  }
}

function Clock(clock, position, address) {
    this.clock = clock;
    this.address = address;
    // console.log(`Clock at ${this.address}`);
    this.position = position;
    this.create();
}

Clock.prototype.create = function () {
    var clock = this.clock;

    this.x = this.position[0] || 0;
    this.y = this.position[1] || 0;

    this.radius = 50;
    this.innerRadius = 46;
    this.ticksRadius = 44;
    this.middleRadius = 5;
    this.hourHandWidth = 10;
    this.hourHandLength = 33;
    this.minuteHandWidth = 10;
    this.minuteHandLength = 43;

    // this.frame = clock.rect( (this.x + 1), (this.y + 1), 60, 60, 0).attr({
    this.frame = clock.circle( (this.x + this.radius), (this.y + this.radius), this.radius).attr({
        fill: "#fff",
        stroke: "#000"
    });

    this.innerFrame = clock.circle( (this.x + this.radius), (this.y + this.radius), this.innerRadius).attr({
        fill: "#FFFFFF00",
        stroke: "#000"
    });

    this.hours = clock.rect( (this.x + this.radius-this.hourHandWidth/2), (this.y + this.radius - this.hourHandLength), this.hourHandWidth, this.hourHandLength).attr({fill: "#333"});

    this.minutes = clock.rect( (this.x + this.radius-this.minuteHandWidth/2), (this.y + this.radius - this.minuteHandLength), this.minuteHandWidth, this.minuteHandLength).attr({fill: "#333"});


    this.middle = clock.circle( ( this.x + this.radius ), ( this.y + this.radius ), this.middleRadius).attr({
        fill: "#ffffff",
        stroke: "#333"
    });

    this.ticks = clock.circle((this.x + this.radius), (this.y + this.radius), this.ticksRadius).attr({
        fill: "#fff",
        stroke: '#333',
        strokeWidth: 3,
        strokeDasharray: '5, 12'
    });


    this.group = clock.group(this.frame, this.hours, this.minutes, this.middle, this.innerFrame);
    // this.id = clock.text(this.x +10, this.y + 50, `${this.address}`);
    this.setTime([getRandomInt(0, 9), getRandomInt(0, 9)]);
};

Clock.prototype.setTime = function(hour, minute) {
  this.hours.animate({transform: "r" + (hour * 30) + "," + ( this.x + this.radius ) + "," + ( this.y + this.radius ) }, 500);
  this.minutes.animate({transform: "r" + (minute * 6) + "," + ( this.x + this.radius ) + "," + ( this.y + this.radius ) }, 500);
};

const CLOCKS_PER_COLUMN = 3
const COLUMNS_PER_GROUP = 2

function ClockGroup(svg, n, width, height) {
    this.svg = svg;
    this.address = START_ADDRESS + n * CLOCKS_PER_COLUMN * COLUMNS_PER_GROUP;
    // console.log(`ClockGroup at ${this.address}`);
    this.width = width;
    this.height = height;
    this.xOffset = n * width + 1;

    this.create();
}

ClockGroup.prototype.create = function() {
  this.clocks = [];
  for ( var column = 0; column < COLUMNS_PER_GROUP; column++) {
    let direction = column%2?-1:1;
    for ( var row = 0; row < CLOCKS_PER_COLUMN; row++) {
      let multiplier = (direction==1)?0:1
      let startvalue = (column * CLOCKS_PER_COLUMN) + (multiplier * (CLOCKS_PER_COLUMN-1));
      let address = this.address + startvalue + direction * row;

      let x = this.xOffset + (column * this.width / COLUMNS_PER_GROUP);

      let yOffset = Math.round(this.height / (CLOCKS_PER_COLUMN-1));
      let y = row * yOffset + 1;
      this.clocks.push( new Clock( this.svg, [x,y], address) );
    }
  }
};

function getRandomInt(min, max) {
    return Math.floor(Math.random() * (max - min)) + min;
}
