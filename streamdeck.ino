#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin Definitions
const int BUTTON_SCENE1 = 2;
const int BUTTON_SCENE2 = 3;
const int BUTTON_MUTE   = 4;
const int BUTTON_CAM    = 5; 
const int POT_PIN       = A0;
const int BUTTON_APP1 = 6;
const int BUTTON_APP2 = 7;

// Button Input Processing States
bool lastScene1State = HIGH;
bool lastScene2State = HIGH;
bool lastMuteState   = HIGH;
bool lastCamState    = HIGH;
bool lastApp1State    = HIGH;
bool lastApp2State    = HIGH;


// System Toggle Variables
bool isAudioOn = true;    
bool isWebcamOn = false;   
String activeStatus = "SYSTEM READY"; // Dynamic text for the top row

int stableVolume = 0;
const int noiseThreshold = 2;

void setup() {
  Serial.begin(9600);

  pinMode(BUTTON_SCENE1, INPUT_PULLUP);
  pinMode(BUTTON_SCENE2, INPUT_PULLUP);
  pinMode(BUTTON_MUTE,   INPUT_PULLUP);
  pinMode(BUTTON_CAM,    INPUT_PULLUP);
  pinMode(BUTTON_APP1,   INPUT_PULLUP);
  pinMode(BUTTON_APP2,   INPUT_PULLUP);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;); 
  }
  
  int rawStart = analogRead(POT_PIN);
  stableVolume = map(rawStart, 0, 1023, 0, 100);
  updateDisplay();
}

void loop() {
  bool changed = false;

  // --- 1. READ ALL PUSH BUTTONS ---
  bool scene1State = digitalRead(BUTTON_SCENE1);
  bool scene2State = digitalRead(BUTTON_SCENE2);
  bool muteState   = digitalRead(BUTTON_MUTE);
  bool camState    = digitalRead(BUTTON_CAM);
  bool app1State   = digitalRead(BUTTON_APP1);
  bool app2State   = digitalRead(BUTTON_APP2);

  if (scene1State == LOW && lastScene1State == HIGH) { 
    Serial.println("SCENE1"); 
    activeStatus = "OBS: SCENE 1"; // Overwrites top line
    changed = true;
    delay(150); 
  }
  lastScene1State = scene1State;

  if (scene2State == LOW && lastScene2State == HIGH) { 
    Serial.println("SCENE2"); 
    activeStatus = "OBS: SCENE 2"; // Overwrites top line
    changed = true;
    delay(150); 
  }
  lastScene2State = scene2State;

  if (muteState == LOW && lastMuteState == HIGH) { 
    Serial.println("MUTE"); 
    isAudioOn = !isAudioOn; 
    activeStatus = isAudioOn ? "MIC: ON" : "MIC: OFF"; // Overwrites top line
    changed = true;
    delay(150); 
  }
  lastMuteState = muteState;

  if (camState == LOW && lastCamState == HIGH) { 
    Serial.println("WEBCAM"); 
    isWebcamOn = !isWebcamOn; 
    activeStatus = isWebcamOn ? "CAM: ON" : "CAM: OFF"; // Overwrites top line
    changed = true;
    delay(150); 
  }
  lastCamState = camState;

  if (app1State == LOW && lastApp1State == HIGH) {
    Serial.println("TELEGRAM");
    activeStatus = "APP: TELEGRAM";
    changed = true;
    delay(150);
  }
  lastApp1State= app1State;

  if (app2State == LOW && lastApp2State == HIGH) {
    Serial.println("SPOTIFY");
    activeStatus = "APP: SPOTIFY";
    changed = true;
    delay(150);
  }
  lastApp1State= app1State;

  // --- 2. READ POTENTIOMETER ---
  int rawPot = analogRead(POT_PIN);
  int currentVolumeReading = map(rawPot, 0, 1023, 0, 100);
  int difference = currentVolumeReading - stableVolume;

  if (abs(difference) >= noiseThreshold) {
    activeStatus = "VOLUME"; // Overwrites top line when tweaking audio knob
    if (difference > 0) {
      for (int i = 0; i < difference; i++) { Serial.println("VOL_UP"); delay(15); }
    } else {
      for (int i = 0; i < abs(difference); i++) { Serial.println("VOL_DOWN"); delay(15); }
    }
    stableVolume = currentVolumeReading;
    changed = true;
  }

  if (changed) {
    updateDisplay();
  }
  delay(10);
}

// Clean, Modular Replacement UI Layout Engine
void updateDisplay() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // --- LINE 1: THE DYNAMIC REPLACING ACTION STATUS (Size 1) ---
  display.setTextSize(1); 
  display.setCursor(0, 0); 
  display.print("STATUS: ");
  display.print(activeStatus); // This string completely handles swaps!
  
  // Sharp structural separation line
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
  
  // --- LINE 2: MAIN VOLUME INTERFACE (Size 3) ---
  display.setTextSize(1);
  display.setCursor(0, 24);
  display.print("LEVEL");

  display.setTextSize(3); 
  display.setCursor(52, 18); 
  display.print(stableVolume);
  
  // --- LINE 3: SLIDER PROGRESS BAR ---
  display.drawRect(0, 52, 128, 10, SSD1306_WHITE);
  int barWidth = map(stableVolume, 0, 100, 0, 124);
  display.fillRect(2, 54, barWidth, 6, SSD1306_WHITE);
  
  display.display();
}