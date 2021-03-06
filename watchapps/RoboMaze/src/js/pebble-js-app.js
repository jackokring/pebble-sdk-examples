function sexyguessimal(coord, letterP, letterM) {
	var letter = (coord < 0)?letterM:letterP;
	coord = Math.abs(coord);
	var degree = Math.floor(coord);
	var minute = Math.floor((coord - degree)*60);
	var second = Math.floor(((coord - degree)*60 - minute)*60);
	return letter+" "+degree.toString()+"°"+minute.toString()+"'"+second.toString()+"\"";
}

function decimal(coord, letterP, letterM) {
	var letter = (coord < 0)?letterM:letterP;
	coord = Math.abs(coord);
	var degree = Math.floor(coord);
	var minute = Math.floor((coord - degree)*10000);
	var d = "0000" + minute.toString();
	return letter+" "+degree.toString()+"."+d.substring(d.length - 4);//leading zero hack
}

function work(latitude, longitude, alt, vel) {
	var long = sexyguessimal(longitude, "E", "W");
	var lat = sexyguessimal(latitude, "N", "S");
	console.log("       "+long+"    "+lat);
        Pebble.sendAppMessage({
	  "NM": Math.floor(vel).toString() + " m/s",
	  "ALT": Math.floor(alt).toString() + " m",
          "DLONG": decimal(longitude, "E", "W"),
          "DLAT": decimal(latitude, "N", "S"),
          "LONG": long,
          "LAT": lat
        },
        function(e) {
    	  console.log('Successfully delivered message with transactionId='
     	   + e.data.transactionId);
  	},
  	function(e) {
          console.log('Unable to deliver message with transactionId='
      	   + e.data.transactionId
           + ' Error is: ' + e.error.message);
  });
}

function locationSuccess(pos) {
  console.log('location ok');
  var coordinates = pos.coords;
  work(coordinates.latitude, coordinates.longitude, coordinates.altitude, coordinates.speed);
}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
  Pebble.sendAppMessage({
    "NM":" Loc Unavail ",
    "ALT":" Loc Unavail ",
    "DLONG":" Loc Unavail ",
    "DLAT":" Loc Unavail ",
    "LONG":" Loc Unavail ",
    "LAT":" Loc Unavail "
  },
  function(e) {
    console.log('Successfully delivered message with transactionId='
      + e.data.transactionId);
  },
  function(e) {
    console.log('Unable to deliver message with transactionId='
      + e.data.transactionId
      + ' Error is: ' + e.error.message);
  });
}

var locationOptions = { "timeout": 15000, "maximumAge": 60000 };
var locationWatcher; 

Pebble.addEventListener("ready", function(e) {
  console.log("connect!" + e.ready);
  locationWatcher = window.navigator.geolocation.watchPosition(locationSuccess, locationError, locationOptions);
  console.log(e.type);
});

Pebble.addEventListener("appmessage", function(e) {
    console.log('Received message: ' + JSON.stringify(e.payload));
});

Pebble.addEventListener('webviewclosed',
  function(e) {
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log('Configuration window returned: ', JSON.stringify(configuration));
  }
);

Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
  Pebble.openURL('https://jackokring.github.io/pebble-sdk-examples/robomaze.html');//you MUST provide a (different) www server
});
