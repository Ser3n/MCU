// html_content.h
const char *html_page = R"=====(
<!DOCTYPE html>
<html>
<style>
    body {
        font-family: fantasy;
        border: 2px solid #ccc;
        padding: 10px;
    }

    h1 {
        text-align: center;
        font-size: 30px;
    }

    p {
        text-align: center;
        font-size: 20px;
    }

    button {
        display: block;
        margin: 20px auto;
        padding: 10px;
        font-size: 16px;
    }

    #RelayStatus {
        display: inline-block;
        width: 20px;
        height: 20px;
    }

    footer {
        margin-top: 20px;
        text-align: center;
    }
</style>

<body>
    <h1>'Erb Monitoring System</h1><br>
    <p>Moisture Level: <span id="MoistureVal">0</span>%</p>
    <p>Humidity: <span id="HumidityVal">0</span>%</p>
    <p>Temperature: <span id="TemperatureVal">0</span>°C</p>
   <p>GPIO 2 - State <span id="buttonStatus">OFF</span></p>
    <button id="PIN2" onclick="togglePin2()">Turn On/Off PIN2</button>


    <footer>
        <p>&copy; 2023 My Webpage. All rights reserved. Zak Turner</p>
        <p id="dateTime"></p>
    </footer>

    <script>
      // Function to toggle Pin2
    function togglePin2() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                // Handle the response if needed
                console.log(this.responseText);
                updateButtonStatus();
            }
        };
        xhttp.open("GET", "togglePin2", true);
        xhttp.send();
    }
//upddate button status
      function updateButtonStatus() {
        var xhttpButtonStatus = new XMLHttpRequest();
        xhttpButtonStatus.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("buttonStatus").innerHTML = this.responseText;
            }
        };
        xhttpButtonStatus.open("GET", "buttonstatus", true);
        xhttpButtonStatus.send();
    }


    // Function to update sensor values
    function updateSensors() {
        // Update Moisture
        var xhttpMoisture = new XMLHttpRequest();
        xhttpMoisture.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("MoistureVal").innerHTML = this.responseText;
            }
        };
        xhttpMoisture.open("GET", "readMoisture", true);
        xhttpMoisture.send();

        // Update Humidity
        var xhttpHumidity = new XMLHttpRequest();
        xhttpHumidity.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("HumidityVal").innerHTML = this.responseText;
            }
        };
        xhttpHumidity.open("GET", "readHumidity", true);
        xhttpHumidity.send();

        // Update Temperature
        var xhttpTemperature = new XMLHttpRequest();
        xhttpTemperature.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("TemperatureVal").innerHTML = this.responseText;
            }
        };
        xhttpTemperature.open("GET", "readTemperature", true);
        xhttpTemperature.send();
    }

    // Periodically update sensor values
    setInterval(updateSensors, 1000);

    document.addEventListener("DOMContentLoaded", function () {
        // Update date and time
        updateDateTime();
    });

    function updateDateTime() {
        var dateTimeElement = document.getElementById("dateTime");
        setInterval(function () {
            var currentDateTime = new Date();
            var formattedDateTime = currentDateTime.toLocaleString();
            dateTimeElement.innerHTML = "Date and Time: " + formattedDateTime;
        }, 1000);
    }
    </script>
</body>

</html>
)=====";