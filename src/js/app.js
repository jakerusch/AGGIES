var myAPIKey = '287396dabce7a142ea95111fe9cc091c';

var temp;

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

// converts time to string
// adjusts for timezone (-5)
function getTime(str) {
  var start = str.split(':')[0].length-2;
  var hour = str.substr(start, 2);
  var returnHour = hour;
  var ampm = ':00am ';
  
  if(hour>5) {
    returnHour = parseInt(hour) - 5;
  } else {
    returnHour = parseInt(hour) + 19;
  }
  
  // convert to 12 hour time
  if(returnHour>12) {
    returnHour = returnHour - 12;
    ampm = ':00pm ';
  }
  
  return returnHour + ampm;
}

function locationSuccess(pos) {
  // Construct URL
  var weatherUrl = "http://api.openweathermap.org/data/2.5/weather?lat=" +
      pos.coords.latitude + "&lon=" + pos.coords.longitude + '&appid=' + myAPIKey + '&units=imperial';  
  
  var forecastUrl = "http://api.openweathermap.org/data/2.5/forecast?lat=" +
      pos.coords.latitude + "&lon=" + pos.coords.longitude + '&appid=' + myAPIKey + '&units=imperial';
//   var weatherUrl = "http://api.openweathermap.org/data/2.5/weather?q=San%20Antonio,tx&appid=" + myAPIKey + "&units=imperial";  
//   var forecastUrl = "http://api.openweathermap.org/data/2.5/forecast?q=San%20Antonio,tx&appid=" + myAPIKey + "&units=imperial";
  
  xhrRequest(weatherUrl, 'GET', 
    function(responseText) {
      var json = JSON.parse(responseText);
            
      // Temperature in Kelvin requires adjustment
      var curTemp = Math.round(json.main.temp);
      console.log("Temperature is " + curTemp);

      // Conditions
      var conditions = json.weather[0].main;      
      console.log("Conditions are " + conditions);      
      
      temp = curTemp + "° " + conditions;
    }
  );

  // Send request to OpenWeatherMap
  xhrRequest(forecastUrl, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      
      // city name
      var city = json.city.name;
      console.log("City is " + city);
      
      ////////////////////////////
      // parse first 3 in array //
      ////////////////////////////
      // first
      var time1 = getTime(json.list[0].dt_txt);
      console.log("Time (1) is " + time1);
      
      var temp1 = Math.round(json.list[0].main.temp) + '° ';
      console.log("Temp (1) is " + temp1);
       
      var weatherDesc1 = json.list[0].weather[0].description;
      console.log("Weather description (1) is " + weatherDesc1);
      
      // second
      var time2 = getTime(json.list[1].dt_txt);
      console.log("Time (2) is " + time2);
      
      var temp2 = Math.round(json.list[1].main.temp) + '° ';
      console.log("Temp (2) is " + temp2);
      
      var weatherDesc2 = json.list[1].weather[0].description;
      console.log("Weather description (2) is " + weatherDesc2);
      
      // third
      var time3 = getTime(json.list[2].dt_txt);
      console.log("Time (3) is " + time3);
      
      var temp3 = Math.round(json.list[2].main.temp) + '° ';
      console.log("Temp (3) is " + temp3);
            
      var weatherDesc3 = json.list[2].weather[0].description;
      console.log("Weather description (3) is " + weatherDesc3);
      
      // assemble dictionary using keys
      var dictionary = {
        "KEY_CITY": city,
        "KEY_TIME1": temp1 + time1,
        "KEY_TEMP1": weatherDesc1,
        "KEY_TIME2": temp2 + time2,
        "KEY_TEMP2": weatherDesc2,
        "KEY_TIME3": temp3 + time3,
        "KEY_TEMP3": weatherDesc3,
        "KEY_TEMP": temp,
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
    }      
  );
}

function locationError(err) {
  console.log("Error requesting location!");
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");

    // Get the initial weather
    getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getWeather();
  }                     
);