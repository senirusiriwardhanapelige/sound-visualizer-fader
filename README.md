# Sound-Reactive Motorized Fader (ESP8266)

<img width="250" height="250" alt="2026-11-08_12-39-29" src="https://github.com/user-attachments/assets/62f23025-b886-42b0-830d-cb51de93c674" />
<img width="250" height="250" alt="2026-11-08_12-39-16" src="https://github.com/user-attachments/assets/c6d09769-f978-4793-98ec-b4943bd73887" />


A slider that moves up and down depending on how loud the sound around it is. Built on a NodeMCU (ESP-12E) with a motor shield. The board hosts its own WiFi network and a local webpage — **no internet or app needed**.


## How it works

1. The ESP8266 creates a WiFi access point called `MotorVisualizer` (password `12345678`).
2. Connect your phone or laptop to that network.
3. Open the local URL shown on the Serial Monitor (e.g. `192.168.4.1`).
4. Click **Start Mic** and allow microphone access.
5. JavaScript in the browser listens to the mic in real time, computes volume, and sends the value over WiFi to the board (`/setTarget?value=…`).
6. The ESP8266 compares the target to the fader's current position (read from the potentiometer on A0) and drives the motor up or down until they match — a physical volume meter.

## Hardware

| Part | Notes |
|------|-------|
| NodeMCU 1.0 (ESP-12E) | Main board |
| ESP12E Motor Shield (L293D H-Bridge) | Drives the fader motor |
| Motorized slide potentiometer | PSM series or similar; built-in pot feeds A0 |
| Jumper wires | Connect board to motor shield |

## Wiring (NodeMCU → Motor Shield)

| NodeMCU pin | GPIO | Motor shield |
|-------------|------|--------------|
| D1 | GPIO5 | PWM / motor speed |
| D2 | GPIO4 | Direction |
| A0 | ADC0 | Potentiometer feedback |

> **Note:** Direction was moved from D3 (GPIO0) to D2 (GPIO4) because GPIO0 is the ESP8266 boot pin — driving it from motor code can interfere with sketch uploads.

## Software setup

1. Install the **ESP8266 board package** in Arduino IDE (Board Manager: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`).
2. Select board: **NodeMCU 1.0 (ESP-12E)**.
3. Open `sound_visualizer_fader.ino`, upload to the board.
4. Open Serial Monitor at 115200 baud to see the AP IP address.
5. Connect to `MotorVisualizer` WiFi and open `http://<IP>` in a browser.

## Repo contents

- `sound_visualizer_fader.ino` — main Arduino sketch
- `README.md` — this file
- `.gitignore` — Arduino/ESP8266 ignore rules
