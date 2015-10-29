var myAPIKey = 'a0b583f6091844ce7283d9c326b249a6';

function iconFromWeatherId(weatherId) {
  if (weatherId < 600) {
    return 2;
  } else if (weatherId < 700) {
    return 3;
  } else if (weatherId > 800) {
    return 1;
  } else {
    return 0;
  }
}

function fetchWeather(latitude, longitude) {
  var req = new XMLHttpRequest();
  req.open('GET', 'http://api.openweathermap.org/data/2.5/weather?' +
    'lat=' + latitude + '&lon=' + longitude + '&cnt=1&appid=' + myAPIKey, true);
  req.onload = function () {
    if (req.readyState === 4) {
      if (req.status === 200) {
        console.log(req.responseText);
        var response = JSON.parse(req.responseText);

        Pebble.sendAppMessage({
          'WeatherKeyIcon': iconFromWeatherId(response.weather[0].id),
          'WeatherKeyTemperature': Math.round(response.main.temp - 273.15) + '\xB0C',
          'WeatherKeyCity': response.name
        });
      } else {
        console.log('Error');
      }
    }
  };
  req.send(null);
}

function locationSuccess(pos) {
  fetchWeather(pos.coords.latitude, pos.coords.longitude);
}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
  Pebble.sendAppMessage({
    'WeatherKeyCity': 'Loc Unavailable',
    'WeatherKeyTemperature': 'N/A'
  });
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(locationSuccess, locationError, {
    'timeout': 15000,
    'maximumAge': 60000
  });
}

Pebble.addEventListener('ready', function (e) {
  getWeather();
});

Pebble.addEventListener('appmessage', function (e) {
  getWeather();
});
