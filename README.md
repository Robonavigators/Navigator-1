# 🚀 Navigator 1: Autonomous Robotics Platform

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT) ![Arduino UNO R4 WiFi](https://img.shields.io/badge/Hardware-Arduino_UNO_R4_WiFi-00979D?style=flat&logo=arduino&logoColor=white)

**Engineered by Team Robonavigators**

Navigator 1 is a highly fault-tolerant, AI-assisted rover powered by a custom 884-line C++ embedded operating system. Built entirely from scratch on the **Arduino UNO R4 WiFi** architecture, it features a standalone asynchronous web server, an active physics-based safety engine, and six intelligent driving modes.

---

## 📊 Project Presentation

**[View the Navigator 1 Technical Presentation & Slide Deck Here](https://drive.google.com/file/d/1NDbsCwXG6vCk6sO1d964J01A3xxocoaC/view?usp=drivesdk)**

This presentation provides a high-level overview of the hardware architecture, the physics-based safety engine, and the custom web OS.

---

## 🌟 Key Features

### 📡 Untethered Web OS
* **Zero-Lag Interface:** Custom HTML/JS dashboard that executes commands and pulls live telemetry via asynchronous fetch requests (no page reloads).
* **PC-Free Boot Sequence:** A physical hardware switch toggles between AP (Wi-Fi broadcast) and STA (Hotspot connection) modes. 
* **Matrix IP Display:** The Arduino’s onboard LED Matrix smoothly scrolls the assigned IP address upon connection, eliminating the need for a serial monitor.
* **Network Failsafes:** A 4-second software heartbeat catches dropped phone connections instantly via a diagnostic NeoPixel. An 8-second boot timeout ensures the robot safely defaults to an offline autonomous mode if Wi-Fi is unavailable.

### 🛡️ Physics & Active Safety Engine
* **Smart Active Braking:** Replaces standard coasting by tracking the motor state and injecting a 50ms reverse-torque pulse to instantly lock the drivetrain and kill skidding momentum.
* **Dual-Sensor Active Shield:** Creates a permanent 25cm virtual bumper. If an object breaches this zone (front or rear), the OS actively overrides human input.
* **Cliff Detection:** Interrupt-level downward IR sensors trigger immediate active braking to prevent table drop-offs.

### 🔋 Power Architecture
* **Custom Charging Matrix:** Proprietary 4-bay TP4056 independent charging station ensures perfectly balanced 18650 lithium-ion cells, eliminating the risks of series charging.
* **Precision Telemetry:** Employs an Exponential Moving Average (EMA) software filter to deliver live, multimeter-accurate battery voltage readings directly to the web UI.

---

## ⚙️ The 6 Intelligent Operating Modes

0. **Smart Idle Shield:** An active parking guard that safely retreats from frontal proximity, checking rear sensors first to avoid backing into walls.
1. **Autopilot:** Utilizes radar-assisted 180-degree scanning to calculate and execute optimal obstacle-avoidance paths.
2. **Draw & Follow:** Translates touchscreen drawings into physical floor vectors. Includes an auto-pause safety feature that freezes execution if an obstacle blocks the path.
3. **Object Follow:** Dynamic spatial tracking that locks onto targets while maintaining a safe 20cm to 80cm distance.
4. **Voice Control:** Integrates the browser's Web Speech API for live, zero-latency continuous voice commands.
5. **Remote Control:** Precision digital joystick navigation protected by the Active Shield in the background.

---

## 🔌 Hardware Wiring & Pinout

| Pin | Connected Component | System Function |
| :--- | :--- | :--- |
| **2** | Motor Driver (IN1) | Left motor direction control |
| **3** | NeoPixel Status LED | Visual diagnostic and network heartbeat indicator |
| **4** | Hardware Toggle Switch | Switches between AP Mode (Wi-Fi) and STA Mode (Hotspot) |
| **5** | Motor Driver (ENA) | Left motor speed control (PWM) |
| **6** | Motor Driver (ENB) | Right motor speed control (PWM) |
| **7** | Front Ultrasonic (ECHO) | Reads front spatial distance |
| **8** | Front Ultrasonic (TRIG) | Triggers front spatial ping |
| **9** | Servo Motor | Sweeps 180° for Autopilot and Object Follow radar |
| **10** | Motor Driver (IN2) | Left motor direction control |
| **11** | Motor Driver (IN3) | Right motor direction control |
| **12** | IR Edge Sensor | Downward-facing cliff and drop-off detection |
| **13** | Motor Driver (IN4) | Right motor direction control |
| **A0** | Analog Voltage Sensor | Reads live raw battery voltage for UI telemetry |
| **A1** | Rear Ultrasonic (ECHO) | Reads rear spatial distance |
| **A2** | Rear Ultrasonic (TRIG) | Triggers rear spatial ping |

---

## 🛠️ Usage Instructions

1. **Booting Up:** * Set the hardware toggle switch on Pin 4 to your desired mode (`HIGH` for Hotspot/STA, `LOW` for internal Wi-Fi/AP).
   * Power on the robot.
2. **Connecting:**
   * Watch the onboard LED matrix. Once connected, it will scroll the IP address (e.g., `192.168.4.1` for AP mode or a dynamic IP for STA mode).
   * Enter that IP address into your smartphone's web browser.
3. **Driving:** * Use the web UI to select your operating mode. 
   * Ensure your phone screen stays active to maintain the software heartbeat and keep the connection alive!

---

## 📄 License

MIT License

Copyright (c) 2026 Robonavigators

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
