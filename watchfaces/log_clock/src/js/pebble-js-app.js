function sexyguessimal(coord, letterP, letterM) {
	var letter = (coord < 0)?letterM:letterP;
	coord = Math.abs(coord);
	var degree = Math.trunc(coord);
	var minute = Math.trunc((coord - degree)*60);
	var second = Math.trunc((coord - degree - 60*minute)*60);
	return letter+" "+degree.toString()+"°"+minute.toString()+"'"+second.toString()+"\"";
}

function work(latitude, longitude) {
	var long = sexyguessimal(longitude, "E", "W");
	var lat = sexyguessimal(latitude, "N", "S");
        Pebble.sendAppMessage({
          "LONG": long,
          "LAT": lat}
        );
}

function locationSuccess(pos) {
  var coordinates = pos.coords;
  work(coordinates.latitude, coordinates.longitude);
}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
  Pebble.sendAppMessage({
    "LONG":" Loc Unavail ",
    "LAT":" Loc Unavail "
  });
}

var locationOptions = { "timeout": 15000, "maximumAge": 60000 }; 

Pebble.addEventListener("ready", function(e) {
  console.log("connect!" + e.ready);
  locationWatcher = window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
  console.log(e.type);
});

Pebble.addEventListener("appmessage", function(e) {
  window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
  console.log(e.type);
  console.log(e.payload.temperature);
  console.log("message!");
});

Pebble.addEventListener("webviewclosed", function(e) {
  console.log("webview closed");
  console.log(e.type);
  console.log(e.response);
});


