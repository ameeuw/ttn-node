function Client() {
  this.rtcTime = undefined;
}

Client.prototype.update = async function () {
  const result = await fetch("/status");
  const data = await result.json();
  console.log(data);
  document.getElementById("status").innerHTML = JSON.stringify(data, null, 2);
};
