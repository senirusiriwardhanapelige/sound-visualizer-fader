/*
 * Sound-Reactive Motorized Fader — ESP8266 / NodeMCU
 *
 * A slider that moves up and down based on surrounding sound volume.
 * The ESP8266 hosts its own WiFi access point ("MotorVisualizer") and
 * serves a local webpage. Connect to the AP, open the page, allow mic
 * access, and the browser streams real-time volume to the board over
 * WiFi. The board compares the target position to the fader's current
 * position (read via the built-in potentiometer on A0) and drives the
 * motor until they match — a physical volume meter, no internet needed.
 *
 * Hardware:
 *   - NodeMCU 1.0 (ESP-12E)
 *   - Motor shield with L293D H-Bridge (or similar)
 *   - Motorized slide potentiometer (PSM series or similar)
 *
 * Pins (NodeMCU D-number mapping):
 *   D1  (GPIO5)  -> PWM motor speed
 *   D2  (GPIO4)  -> Motor direction (was D3/GPIO0 — moved because GPIO0
 *                  is the ESP8266 boot pin; driving it can break uploads)
 *   A0  (ADC0)   -> Potentiometer feedback (fader position)
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

const int pwmMotorA = D1;   // GPIO5 — motor speed (PWM)
const int dirMotorA = D2;   // GPIO4 — motor direction

int TARGET = 0;   // target position from the webpage (0-1023)
int current = 0;  // current fader position from potentiometer (0-1023)

// ------------------------------------------------------------
// Webpage served by the ESP8266 (runs entirely on the board)
// ------------------------------------------------------------
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Sound Visualizer</title>
  <style>
    body { font-family: sans-serif; padding: 2rem; max-width: 480px; }
    button { padding: 0.75rem 1.5rem; font-size: 1rem; cursor: pointer; }
    #status, #value { margin: 0.5rem 0; font-size: 1.1rem; }
  </style>
</head>
<body>
  <h2>Sound Visualizer</h2>
  <button onclick="start()">Start Mic</button>
  <p id="status">Not started</p>
  <p id="value">Value: 0</p>

  <script>
    function start() {
      navigator.mediaDevices.getUserMedia({ audio: true })
        .then(stream => {
          document.getElementById("status").innerText = "Listening…";
          const audioCtx = new AudioContext();
          const analyser = audioCtx.createAnalyser();
          const source = audioCtx.createMediaStreamSource(stream);
          source.connect(analyser);
          analyser.fftSize = 256;
          const data = new Uint8Array(analyser.frequencyBinCount);

          let smoothed = 0;

          setInterval(() => {
            analyser.getByteTimeDomainData(data);
            let sum = 0;
            for (let i = 0; i < data.length; i++) {
              let v = (data[i] - 128) / 128;
              sum += v * v;
            }
            let rms = Math.sqrt(sum / data.length);

            smoothed = smoothed * 0.8 + (rms * 1023) * 0.2;
            let value = Math.round(smoothed * 8); // amplify, tune this number

            if (value > 1023) value = 1023;

            document.getElementById("value").innerText = "Value: " + value;
            fetch('/setTarget?value=' + value);
          }, 100);
        })
        .catch(err => {
          document.getElementById("status").innerText = "Mic blocked: " + err;
        });
    }
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleSetTarget() {
  if (server.hasArg("value")) {
    TARGET = server.arg("value").toInt();
    Serial.println(TARGET);
  }
  server.send(200, "text/plain", "ok");
}

void setup() {
  Serial.begin(115200);
  pinMode(pwmMotorA, OUTPUT);
  pinMode(dirMotorA, OUTPUT);

  // ESP8266 creates its own WiFi access point — no internet required
  WiFi.softAP("MotorVisualizer", "12345678");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/setTarget", handleSetTarget);
  server.begin();
}

unsigned long lastMove = 0;

void loop() {
  server.handleClient();

  // Control loop runs every 20 ms
  if (millis() - lastMove > 20) {
    lastMove = millis();
    current = analogRead(A0);

    if (current < TARGET - 20) {
      digitalWrite(dirMotorA, LOW);
      analogWrite(pwmMotorA, 200);
    } else if (current > TARGET + 20) {
      digitalWrite(dirMotorA, HIGH);
      analogWrite(pwmMotorA, 200);
    } else {
      analogWrite(pwmMotorA, 0); // at target — stop motor
    }
  }
}
