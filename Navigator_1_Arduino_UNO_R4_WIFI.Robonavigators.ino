#include <WiFiS3.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>
#include <Servo.h>

int ENA = 5;
int IN1 = 13;
int IN2 = 11;
int IN3 = 10;
int IN4 = 2;
int ENB = 6;
int edgeSensor = 12;
int statusLed = 3; 
int wifiModePin = 4;   
int echoPin = 7;       
int trigPin = 8;       
int rearEchoPin = A1;  
int rearTrigPin = A2;  
int servoPin = 9;
int voltSensor = A0;
int motorSpeed = 200;
int currentSpeed = 150;
int numPixels = 1;
int brightness = 127;

int clientCount = 0;
int webPageOpen = 0;
int cautionTriggered = 0;
int currentMode = 0;
int dist = 0;
int duration = 0;
int pathIndex = 0;
int pathTimer = 0;
int pathStepDuration = 500; 
int manualCmd = 0;
int isStopped = 1;
int isSTAMode = 0; 

// --- Tracks which way the robot is moving for Smart Braking ---
int lastMotorState = 0; // 1=F, 2=B, 3=L, 4=R

// --- NEW: Tracks the Web OS pings to detect disconnections instantly ---
unsigned long lastHeartbeat = 0; 

float smoothedVoltage = 0; 

String activePath = "";

// --- AP CREDENTIALS (Pin 4 to GND) ---
char ssid_AP[] = "Navigator1 by Robonavigators";
char pass_AP[] = "Navigator1@Robonavigators";

// --- STA/HOTSPOT CREDENTIALS (Pin 4 to 5V) ---
char ssid_STA[] = "Xiaomi 11i Rayyan"; 
char pass_STA[] = "Robonavigators@Stem";

WiFiServer server(80);
Adafruit_NeoPixel strip(numPixels, statusLed, NEO_GRB + NEO_KHZ800);
ArduinoLEDMatrix matrix;
Servo steeringServo;

String htmlPage = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
<style>
body { background-color: #0484f8; color: #0a0a0a; font-family: system-ui, -apple-system, sans-serif; text-align: center; margin: 0; padding: 15px; touch-action: manipulation; overflow-x: hidden; }
h2 { font-family: 'Courier New', Courier, monospace; color: #000000; font-size: 24px; font-weight: bold; letter-spacing: 2px; text-transform: uppercase; margin-bottom: 15px; }

.status-panel { background: #1a1a1a; border-left: 6px solid #000000; border-radius: 8px; padding: 15px; margin: 0 auto 15px auto; width: 85%; font-family: 'Courier New', Courier, monospace; font-weight: bold; font-size: 15px; color: #0484f8; box-shadow: 0 6px 15px rgba(0, 0, 0, 0.4); }

.telemetry { display: flex; justify-content: center; gap: 15px; margin-bottom: 15px; }
.telemetry-box { background: #1a1a1a; border: 2px solid #000000; padding: 10px; border-radius: 10px; font-family: 'Courier New', Courier, monospace; font-size: 14px; color: #cccccc; box-shadow: 0 4px 10px rgba(0, 0, 0, 0.3); display: flex; align-items: center; justify-content: center; width: 60%; }
.telemetry-box span.val { color: #0484f8; font-weight: bold; font-size: 16px; margin-left: 4px; }

.slider-container { background: #1a1a1a; padding: 15px; border-radius: 12px; width: 80%; margin: 0 auto 15px auto; border: 2px solid #000000; box-shadow: 0 4px 10px rgba(0, 0, 0, 0.3); }
.slider { width: 100%; accent-color: #0484f8; }
.pwmVal { font-family: 'Courier New', Courier, monospace; font-weight: bold; margin-top: 10px; font-size: 14px; color: #0484f8; }

.tile-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 15px; width: 90%; margin: 0 auto 20px auto; }
.tile { background: linear-gradient(145deg, #1a1a1a, #0d0d0d); color: #0484f8; border: 2px solid #000000; border-radius: 15px; aspect-ratio: 1; display: flex; flex-direction: column; align-items: center; justify-content: center; font-family: 'Courier New', Courier, monospace; font-size: 18px; font-weight: bold; text-transform: uppercase; box-shadow: 0 6px 12px rgba(0, 0, 0, 0.4); transition: 0.1s; cursor: pointer; text-align: center; }
.tile:active { background: #000000; color: #ffffff; transform: scale(0.95); }

.icon-svg { width: 38px; height: 38px; stroke: currentColor; stroke-width: 2; fill: none; margin-bottom: 10px; stroke-linecap: round; stroke-linejoin: round; }
.tiny-svg { width: 18px; height: 18px; stroke: currentColor; stroke-width: 2; fill: none; margin-left: 6px; stroke-linecap: round; stroke-linejoin: round; }

.screen { display: none; flex-direction: column; align-items: center; width: 100%; animation: fadeIn 0.3s; }
#mainScreen { display: flex; }
@keyframes fadeIn { from { opacity: 0; transform: scale(0.98); } to { opacity: 1; transform: scale(1); } }

.back-btn { background: none; border: none; color: #000000; font-family: 'Courier New', Courier, monospace; font-weight: bold; font-size: 18px; width: 90%; text-align: left; padding: 10px 0; margin-bottom: 20px; cursor: pointer; }
canvas { border-radius: 12px; background-color: #1a1a1a; border: 3px solid #000000; touch-action: none; display: block; box-shadow: 0 8px 25px rgba(0, 0, 0, 0.5); }

#joyContainer { width: 200px; height: 200px; border-radius: 50%; background: #1a1a1a; border: 3px solid #000000; position: relative; box-shadow: inset 0 0 30px rgba(0,0,0,0.8), 0 8px 25px rgba(0,0,0,0.5); margin-top: 15px; }
#joyKnob { width: 80px; height: 80px; border-radius: 50%; background: #0484f8; position: absolute; top: 60px; left: 60px; border: 2px solid #000000; box-shadow: 0 0 20px rgba(0,0,0,0.6); pointer-events: none; }

.voice-box { background: #1a1a1a; border: 3px solid #000000; border-radius: 12px; padding: 20px; width: 80%; box-shadow: 0 8px 25px rgba(0, 0, 0, 0.5); margin-top: 20px; }
.voice-text { font-family: 'Courier New', monospace; font-weight: bold; color: #0484f8; font-size: 18px; }
.pulse { animation: pulse 1.5s infinite; stroke: #0484f8 !important; }
@keyframes pulse { 0% { transform: scale(1); } 50% { transform: scale(1.15); } 100% { transform: scale(1); } }
</style>
</head>
<body>

<div id="mainScreen" class="screen">
  <h2>Navigator 1 OS</h2>
  <div class="status-panel" id="statusBox">SYSTEM: IDLE SHIELD</div>
  
  <div class="telemetry">
    <div class="telemetry-box">
      BATTERY
      VOLTAGE: <span id="vol" class="val"></span>V 
      <svg class="tiny-svg" viewBox="0 0 24 24">Wait...<path d="M13 2L3 14h9l-1 8 10-12h-9l1-8z"/></svg>
    </div>
  </div>

  <div class="slider-container">
    <input type="range" min="1" max="100" value="35" class="slider sync-slider" onchange="sendPWM(this.value)" oninput="updateSliderUI(this.value)">
    <div class="pwmVal">SPEED CONTROLLER: 35%</div>
  </div>

  <div class="tile-grid">
    <div class="tile" onclick="setMode(1)">
      <svg class="icon-svg" viewBox="0 0 24 24"><circle cx="12" cy="12" r="10"/><path d="M12 2v4m0 12v4m10-10h-4M6 12H2"/></svg>
      Autopilot
    </div>
    <div class="tile" onclick="openScreen('drawScreen', 2)">
      <svg class="icon-svg" viewBox="0 0 24 24"><circle cx="6" cy="18" r="3"/><circle cx="18" cy="6" r="3"/><path d="M8 16l8-8"/></svg>
      Draw and follow
    </div>
    <div class="tile" onclick="setMode(3)">
      <svg class="icon-svg" viewBox="0 0 24 24"><path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2"/><circle cx="12" cy="7" r="4"/></svg>
      Object Follow
    </div>
    <div class="tile" onclick="openVoiceScreen()">
      <svg class="icon-svg" viewBox="0 0 24 24"><path d="M12 3v18m-4-12v6m8-6v6m-12-2v2m16-2v2"/></svg>
      Voice Control
    </div>
    <div class="tile" style="grid-column: span 2; aspect-ratio: auto; padding: 20px;" onclick="openScreen('joyScreen', 5)">
      <svg class="icon-svg" viewBox="0 0 24 24" style="margin-bottom: 5px;"><circle cx="12" cy="12" r="10"/><circle cx="12" cy="12" r="3"/><path d="M12 2v7m0 6v7m10-10h-7M2 12h7"/></svg>
      Remote Control
    </div>
  </div>
</div>

<div id="drawScreen" class="screen">
  <button class="back-btn" onclick="goBack()">&lt; SYSTEM RETURN</button>
  <h2>DRAW AND FOLLOW</h2>
  
  <div class="slider-container" style="margin-bottom: 20px;">
    <input type="range" min="1" max="100" value="35" class="slider sync-slider" onchange="sendPWM(this.value)" oninput="updateSliderUI(this.value)">
    <div class="pwmVal">SPEED CONTROLLER: 35%</div>
  </div>
  
  <p style="font-family: monospace; color: #000000; font-weight: bold; font-size: 14px; margin-bottom: 20px;">TRACE PATH ON RADAR TO EXECUTE</p>
  <canvas id="canvas" width="300" height="300"></canvas>
</div>

<div id="joyScreen" class="screen">
  <button class="back-btn" onclick="goBack()">&lt; SYSTEM RETURN</button>
  <h2>REMOTE CONTROL</h2>
  
  <div class="slider-container">
    <input type="range" min="1" max="100" value="35" class="slider sync-slider" onchange="sendPWM(this.value)" oninput="updateSliderUI(this.value)">
    <div class="pwmVal">SPEED CONTROLLER: 35%</div>
  </div>
  
  <div id="joyContainer">
    <div id="joyKnob"></div>
  </div>
</div>

<div id="voiceScreen" class="screen">
  <button class="back-btn" onclick="goBack()">&lt; SYSTEM RETURN</button>
  <h2>VOICE CONTROL</h2>
  <p style="font-family: monospace; color: #000000; font-weight: bold; font-size: 14px;">TAP MIC TO SPEAK</p>
  
  <div style="margin-top: 30px;">
    <svg id="micIcon" class="icon-svg" style="width: 60px; height: 60px; stroke: #000000; cursor: pointer;" viewBox="0 0 24 24" onclick="startVoice()"><path d="M12 3v18m-4-12v6m8-6v6m-12-2v2m16-2v2"/></svg>
  </div>
  
  <div class="voice-box">
    <div id="voiceOutput" class="voice-text">[ STANDBY ]</div>
  </div>
</div>

<script>
function openScreen(screenId, modeNum) {
  document.getElementById('mainScreen').style.display = 'none';
  document.getElementById(screenId).style.display = 'flex';
  setMode(modeNum);
}

function goBack() {
  document.getElementById('drawScreen').style.display = 'none';
  document.getElementById('joyScreen').style.display = 'none';
  document.getElementById('voiceScreen').style.display = 'none';
  document.getElementById('mainScreen').style.display = 'flex';
  
  document.getElementById('micIcon').classList.remove('pulse');
  sendCmd('S');
  setMode(0);
}

function openVoiceScreen() {
  openScreen('voiceScreen', 4);
}

function updateSliderUI(val) {
  let sliders = document.querySelectorAll('.sync-slider');
  let labels = document.querySelectorAll('.pwmVal');
  sliders.forEach(s => s.value = val);
  labels.forEach(l => l.innerHTML = "SPEED CONTROLLER: " + val + "%");
}

function sendPWM(val) { 
  updateSliderUI(val);
  let safePWM = Math.floor(100 + ((val - 1) * 1.565));
  fetch('/?pwm=' + safePWM); 
}
function sendCmd(c) { fetch('/?cmd=' + c); }

function setMode(m) { 
  fetch('/?mode=' + m); 
  let modes = ["IDLE SHIELD", "AUTOPILOT", "DRAW AND FOLLOW", "OBJECT FOLLOW", "VOICE CONTROL", "REMOTE CONTROL"];
  document.getElementById("statusBox").innerText = "SYSTEM: " + modes[m];
}

function startVoice() {
  let vOut = document.getElementById("voiceOutput");
  let mic = document.getElementById("micIcon");

  window.SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
  
  if (!window.SpeechRecognition) {
    vOut.innerText = "[ BROWSER NOT SUPPORTED. USE CHROME/SAFARI ]";
    return;
  }

  let recognition = new SpeechRecognition();
  recognition.continuous = true; 
  recognition.interimResults = true; 

  let lastCmdTime = 0; 

  recognition.onstart = function() {
    mic.classList.add('pulse');
    vOut.innerText = "[ LISTENING LIVE... ]";
  };

  recognition.onresult = function(event) {
    let transcript = event.results[event.results.length - 1][0].transcript.toLowerCase();
    
    if (Date.now() - lastCmdTime < 600) return; 

    let matched = true;
    
    if (transcript.includes('left')) { fetch('/?voice=L'); vOut.innerText = "EXECUTING: LEFT"; }
    else if (transcript.includes('right')) { fetch('/?voice=R'); vOut.innerText = "EXECUTING: RIGHT"; }
    else if (transcript.includes('forward')) { fetch('/?cmd=F'); vOut.innerText = "EXECUTING: FORWARD"; }
    else if (transcript.includes('back')) { fetch('/?cmd=B'); vOut.innerText = "EXECUTING: BACK"; }
    else if (transcript.includes('stop')) { fetch('/?cmd=S'); vOut.innerText = "EXECUTING: STOP"; }
    else if (transcript.includes('autopilot')) { fetch('/?mode=1'); setMode(1); }
    else { matched = false; }

    if (matched) {
      lastCmdTime = Date.now(); 
    }
  };

  recognition.onerror = function(event) {
    vOut.innerText = "[ ERROR: " + event.error.toUpperCase() + " ]";
    mic.classList.remove('pulse');
  };

  recognition.onend = function() {
    mic.classList.remove('pulse');
    setTimeout(() => { 
      if(!vOut.innerText.includes("ERROR")) {
        vOut.innerText = "[ STANDBY ]"; 
      }
    }, 2000);
  };

  recognition.start();
}

setInterval(function(){ 
  fetch('/ping')
    .then(res => res.text())
    .then(data => {
      if(data) {
        document.getElementById('vol').innerText = data;
      }
    }).catch(err => console.log(err));
}, 1500);

let canvas = document.getElementById("canvas");
let ctx = canvas.getContext("2d");
let isDrawing = false; let lastX = 0; let lastY = 0; let pathStr = "";

canvas.addEventListener("touchstart", function(e) {
  isDrawing = true; let rect = canvas.getBoundingClientRect();
  lastX = e.touches[0].clientX - rect.left; lastY = e.touches[0].clientY - rect.top;
  pathStr = ""; ctx.clearRect(0, 0, canvas.width, canvas.height);
  ctx.beginPath(); ctx.moveTo(lastX, lastY); ctx.strokeStyle = "#0484f8"; ctx.lineWidth = 4;
});
canvas.addEventListener("touchmove", function(e) {
  if(!isDrawing) return; e.preventDefault();
  let rect = canvas.getBoundingClientRect();
  let x = e.touches[0].clientX - rect.left; let y = e.touches[0].clientY - rect.top;
  ctx.lineTo(x, y); ctx.stroke();
  let dx = x - lastX; let dy = y - lastY;
  
  if(Math.abs(dx) > 20 || Math.abs(dy) > 20) {
    if(Math.abs(dx) > Math.abs(dy)) { 
      pathStr += (dx > 0) ? "R" : "L"; 
    } else { 
      pathStr += "F"; 
    }
    lastX = x; lastY = y;
  }
});
canvas.addEventListener("touchend", function(e) { isDrawing = false; fetch('/?path=' + pathStr); });

let joy = document.getElementById("joyContainer");
let knob = document.getElementById("joyKnob");
let joyActive = false; let currentCmd = 'S';

joy.addEventListener("touchstart", handleJoy, {passive: false});
joy.addEventListener("touchmove", handleJoy, {passive: false});
joy.addEventListener("touchend", function(e) {
  joyActive = false; knob.style.transform = `translate(0px, 0px)`;
  if(currentCmd !== 'S') { sendCmd('S'); currentCmd = 'S'; }
});

function handleJoy(e) {
  e.preventDefault(); joyActive = true;
  let rect = joy.getBoundingClientRect();
  let x = e.touches[0].clientX - rect.left - 100;
  let y = e.touches[0].clientY - rect.top - 100;
  
  let dist = Math.sqrt(x*x + y*y);
  if(dist > 60) { x = (x/dist) * 60; y = (y/dist) * 60; }
  knob.style.transform = `translate(${x}px, ${y}px)`;
  
  let newCmd = 'S';
  if(dist > 20) {
    if(Math.abs(x) > Math.abs(y)) newCmd = x > 0 ? 'R' : 'L';
    else newCmd = y > 0 ? 'B' : 'F';
  }
  
  if(newCmd !== currentCmd) { sendCmd(newCmd); currentCmd = newCmd; }
}
</script>
</body>
</html>
)=====";

// --- SMART ACTIVE BRAKE SYSTEM ---
void stopMotors() {
  if (isStopped == 1) return;
  
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  delay(50);
  
  if (lastMotorState == 1) { 
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  } 
  else if (lastMotorState == 2) { 
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  } 
  else if (lastMotorState == 3) { 
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  } 
  else if (lastMotorState == 4) { 
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  }
  
  analogWrite(ENA, 255);
  analogWrite(ENB, 255);
  delay(50);
  
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  isStopped = 1;
}

void moveForward() {
  isStopped = 0;
  lastMotorState = 1; 
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, currentSpeed);
  analogWrite(ENB, currentSpeed);
}

void moveBackward() {
  isStopped = 0;
  lastMotorState = 2; 
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, currentSpeed);
  analogWrite(ENB, currentSpeed);
}

void turnLeft() {
  isStopped = 0;
  lastMotorState = 3; 
  int safeSpeed = currentSpeed;
  if (safeSpeed > 150) safeSpeed = 150;
  
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  
  analogWrite(ENA, safeSpeed);
  analogWrite(ENB, safeSpeed);
}

void turnRight() {
  isStopped = 0;
  lastMotorState = 4; 
  int safeSpeed = currentSpeed;
  if (safeSpeed > 150) safeSpeed = 150;
  
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  
  analogWrite(ENA, safeSpeed);
  analogWrite(ENB, safeSpeed);
}

float getVoltage() {
  int sensorValue = analogRead(voltSensor);
  float rawVoltage = (sensorValue * 25.0) / 1023.0;
  
  float calibrationFactor = 1.00; 
  float instantVoltage = rawVoltage * calibrationFactor;
  
  if (smoothedVoltage == 0) {
    smoothedVoltage = instantVoltage; 
  } else {
    smoothedVoltage = (smoothedVoltage * 0.98) + (instantVoltage * 0.02);
  }
  
  return smoothedVoltage;
}

// --- DIAGNOSTIC LED SYSTEM (WITH SOFTWARE HEARTBEAT) ---
void updateLED() {
  float voltage = getVoltage();

  if (millis() < 3000) {
    if (voltage >= 7.4) strip.setPixelColor(0, strip.Color(0, 255, 0));
    else if (voltage >= 6.8) strip.setPixelColor(0, strip.Color(255, 255, 0));
    else strip.setPixelColor(0, strip.Color(255, 0, 0));
    strip.show();
    return;
  }

  if (voltage < 6.4) {
    strip.setPixelColor(0, strip.Color(255, 0, 0));
    strip.show();
    return;
  }

  unsigned long currentMillis = millis();
  bool isBlinkOn = (currentMillis / 500) % 2 == 0; 
  
  // If we haven't heard from the phone in 4 seconds, it disconnected!
  bool hasActiveClient = (currentMillis - lastHeartbeat < 4000);

  if (isSTAMode == 1) {
    if (WiFi.status() == WL_CONNECTED && hasActiveClient) {
      strip.setPixelColor(0, strip.Color(0, 255, 0)); 
    } else {
      if (isBlinkOn) strip.setPixelColor(0, strip.Color(255, 0, 0)); 
      else strip.setPixelColor(0, strip.Color(0, 0, 0));
    }
  } else {
    if (hasActiveClient) {
      strip.setPixelColor(0, strip.Color(0, 255, 0)); 
    } else {
      if (isBlinkOn) strip.setPixelColor(0, strip.Color(0, 0, 255)); 
      else strip.setPixelColor(0, strip.Color(0, 0, 0));
    }
  }
  
  strip.show();
}

void checkEdge() {
  if (digitalRead(edgeSensor) == 1) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, 100);
    analogWrite(ENB, 100);
    delay(150);
    if (digitalRead(edgeSensor) == 1) {
      analogWrite(ENA, 255);
      analogWrite(ENB, 255);
    }
    while (digitalRead(edgeSensor) == 1) delay(10);
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
  }
}

int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH, 10000); 
  
  if (duration == 0) return 100;
  dist = duration * 0.034 / 2;
  return dist;
}

int getRearDistance() {
  digitalWrite(rearTrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(rearTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(rearTrigPin, LOW);
  
  long durationRear = pulseIn(rearEchoPin, HIGH, 10000); 
  
  if (durationRear == 0) return 100;
  int dRear = durationRear * 0.034 / 2;
  return dRear;
}

int lookLeft() {
  steeringServo.write(180);
  delay(400);
  int leftDist = getDistance();
  return leftDist;
}

int lookRight() {
  steeringServo.write(0);
  delay(600); 
  int rightDist = getDistance();
  return rightDist;
}

void runAutopilot() {
  int d = getDistance();
  
  steeringServo.write(90); 

  if (d > 0 && d < 20) {
    stopMotors();
    
    int distLeft = lookLeft();
    int distRight = lookRight();
    
    steeringServo.write(90);
    delay(300);

    if (distLeft >= distRight) {
      turnLeft();
      delay(1000); 
    } else {
      turnRight();
      delay(1000); 
    }
    
    stopMotors();
  } else {
    moveForward();
  }
}

// --- SMART IDLE SHIELD WITH REAR SENSOR CHECK ---
void runIdleShield() {
  int frontDist = getDistance();
  
  if (frontDist > 0 && frontDist < 15) {
    int rearDist = getRearDistance();
    
    if (rearDist == 0 || rearDist > 20) {
      moveBackward();
      delay(400);
      stopMotors();
    } else {
      stopMotors();
    }
  }
}

void runObjectFollow() {
  int d = getDistance();
  
  steeringServo.write(90);

  if (d > 20 && d < 80) {
    moveForward();
  } 
  else if (d > 0 && d <= 20) {
    stopMotors();
  } 
  else {
    stopMotors(); 
    
    int leftDist = lookLeft();
    int rightDist = lookRight();
    
    steeringServo.write(90); 
    delay(200);

    if (leftDist > 0 && leftDist < 80) {
      turnLeft();
      delay(1000); 
      stopMotors();
    }
    else if (rightDist > 0 && rightDist < 80) {
      turnRight();
      delay(1000); 
      stopMotors();
    }
  }
}

// --- ACTIVE SAFETY SHIELD ADDED TO DRAW AND FOLLOW ---
void runDrawFollow() {
  if (activePath.length() == 0 || pathIndex >= activePath.length()) {
    stopMotors();
    return;
  }
  
  steeringServo.write(90); 
  int frontDist = getDistance();
  
  if (frontDist > 0 && frontDist < 25) {
    stopMotors(); 
    pathTimer = millis(); // Freezes the execution step timer so it doesn't skip moves
    return; 
  }

  if (millis() - pathTimer > pathStepDuration) {
    char step = activePath.charAt(pathIndex);
    if (step == 'F') moveForward();
    if (step == 'B') moveBackward(); 
    if (step == 'L') turnLeft();
    if (step == 'R') turnRight();
    pathIndex++;
    pathTimer = millis();
  }
}

void runManualWithProtection() {
  if (manualCmd == 1) { 
    steeringServo.write(90); 
    int currentDist = getDistance();
    if (currentDist > 0 && currentDist < 25) {
      stopMotors();
      manualCmd = 0; 
    } else {
      moveForward();
    }
  } 
  else if (manualCmd == 2) { 
    steeringServo.write(90); 
    int currentRearDist = getRearDistance();
    if (currentRearDist > 0 && currentRearDist < 25) {
      stopMotors();
      manualCmd = 0;
    } else {
      moveBackward(); 
    }
  } 
  else if (manualCmd == 3) {
    turnLeft();
  } 
  else if (manualCmd == 4) {
    turnRight();
  } 
  else {
    stopMotors();
  }
}

void setup() {
  Serial.begin(9600); 
  
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(edgeSensor, INPUT);
  
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  
  pinMode(rearEchoPin, INPUT);
  pinMode(rearTrigPin, OUTPUT);
  
  pinMode(voltSensor, INPUT);
  pinMode(wifiModePin, INPUT); 

  strip.begin();
  strip.setBrightness(brightness);
  strip.show();

  matrix.begin();
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(50);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.print(" NAVIGATOR 1 ");
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();

  steeringServo.attach(servoPin);
  steeringServo.write(90);

  // --- HARDWARE SWITCH LOGIC WITH MATRIX IP SCROLL ---
  if (digitalRead(wifiModePin) == HIGH) {
    isSTAMode = 1;
    Serial.println("MODE: STA (Connecting to Hotspot)");
    
    WiFi.begin(ssid_STA, pass_STA);
    
    int connectionAttempts = 0;
    while (WiFi.status() != WL_CONNECTED && connectionAttempts < 16) {
      delay(500);
      Serial.print(".");
      strip.setPixelColor(0, strip.Color(255, 0, 0)); 
      strip.show();
      delay(500);
      strip.setPixelColor(0, strip.Color(0, 0, 0));
      strip.show();
      connectionAttempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("Connected to Hotspot!");
      Serial.print("ROBOT IP ADDRESS: ");
      Serial.println(WiFi.localIP()); 
      
      // Build IP string to scroll on Matrix
      IPAddress ip = WiFi.localIP();
      String ipStr = " IP: " + String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]) + " ";
      
      matrix.beginDraw();
      matrix.stroke(0xFFFFFFFF);
      matrix.textScrollSpeed(50);
      matrix.textFont(Font_5x7);
      matrix.beginText(0, 1, 0xFFFFFF);
      matrix.print(ipStr);
      matrix.endText(SCROLL_LEFT);
      matrix.endDraw();
      
    } else {
      Serial.println("");
      Serial.println("Hotspot not found. Proceeding to OFFLINE MODE.");
    }
  } 
  else {
    isSTAMode = 0;
    Serial.println("MODE: AP (Broadcasting Wi-Fi)");
    WiFi.beginAP(ssid_AP, pass_AP);
    Serial.println("AP Created!");
    Serial.print("ROBOT IP ADDRESS: 192.168.4.1");
    
    // Scroll static AP address on Matrix
    matrix.beginDraw();
    matrix.stroke(0xFFFFFFFF);
    matrix.textScrollSpeed(50);
    matrix.textFont(Font_5x7);
    matrix.beginText(0, 1, 0xFFFFFF);
    matrix.print(" IP: 192.168.4.1 ");
    matrix.endText(SCROLL_LEFT);
    matrix.endDraw();
  }

  server.begin();
}

void loop() {
  updateLED();
  checkEdge();

  if (currentMode == 0) runIdleShield();
  if (currentMode == 1) runAutopilot();
  if (currentMode == 2) runDrawFollow();
  if (currentMode == 3) runObjectFollow();
  
  if (currentMode == 4 || currentMode == 5) {
    runManualWithProtection();
  }

  WiFiClient client = server.available();
  if (client) {
    client.setTimeout(20); 
    String request = client.readStringUntil('\r');
    
    if (request.indexOf("GET /ping") != -1) {
      lastHeartbeat = millis(); // <-- TRIGGERS LED CONNECTION STATUS
      webPageOpen = 1;
      client.flush();
      client.println("HTTP/1.1 200 OK");
      client.println("Connection: close"); 
      client.println("");
      client.print(getVoltage()); 
      client.stop();
      return;
    }

    if (request.indexOf("GET /?") != -1) {
      lastHeartbeat = millis(); // <-- TRIGGERS LED CONNECTION STATUS
      webPageOpen = 1;
      
      if (request.indexOf("pwm=") != -1) {
        int pwmStart = request.indexOf("pwm=") + 4;
        int pwmEnd = request.indexOf(" ", pwmStart);
        currentSpeed = request.substring(pwmStart, pwmEnd).toInt();
      }
      if (request.indexOf("mode=") != -1) {
        int modeStart = request.indexOf("mode=") + 5;
        currentMode = request.substring(modeStart, modeStart + 1).toInt();
        stopMotors();
      }
      if (request.indexOf("path=") != -1) {
        int pathStart = request.indexOf("path=") + 5;
        int pathEnd = request.indexOf(" ", pathStart);
        activePath = request.substring(pathStart, pathEnd);
        pathIndex = 0;
        pathTimer = millis();
      }
      if (request.indexOf("cmd=") != -1) {
        if (request.indexOf("cmd=F") != -1) manualCmd = 1;
        if (request.indexOf("cmd=B") != -1) manualCmd = 2;
        if (request.indexOf("cmd=L") != -1) manualCmd = 3;
        if (request.indexOf("cmd=R") != -1) manualCmd = 4;
        if (request.indexOf("cmd=S") != -1) manualCmd = 0;
      }

      if (request.indexOf("voice=L") != -1) { turnLeft(); delay(1000); stopMotors(); manualCmd = 0; }
      if (request.indexOf("voice=R") != -1) { turnRight(); delay(1000); stopMotors(); manualCmd = 0; }

      client.flush();
      client.println("HTTP/1.1 200 OK");
      client.println("Connection: close");
      client.println("");
      client.print("OK");
      client.stop();
    }
    else if (request.indexOf("GET / ") != -1) {
      webPageOpen = 1;
      client.flush();
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      client.println("");
      client.println(htmlPage);
      client.stop();
    }
  }
}