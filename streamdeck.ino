#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const uint8_t BUTTON_PINS[6] = {2, 3, 4, 5, 6, 7};
const uint8_t KEY_NUMBERS[6] = {13, 14, 15, 16, 17, 18};
const char* DEFAULT_LABELS[6] = {"S1", "S2", "CAM", "VC", "SPT", "TEL"};

// Indices of the two toggle-style buttons (CAM and VC)
const uint8_t CAM_INDEX = 2;
const uint8_t VC_INDEX = 3;

bool camToggleOn = false;
bool vcToggleOn = false;
bool camShowingToggle = false; // true = currently showing ON/OFF instead of "CAM"
bool vcShowingToggle = false;  // true = currently showing ON/OFF instead of "VC"

bool lastState[6] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
unsigned long lastDebounceTime[6] = {0, 0, 0, 0, 0, 0};
const unsigned long DEBOUNCE_MS = 25;

const int POT_PIN = A0;
int lastVolPercent = -1;
unsigned long lastPotRead = 0;
const unsigned long POT_READ_INTERVAL = 50;

void setup() {
  Serial.begin(115200);

  for (uint8_t i = 0; i < 6; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    // If display doesn't init, halt with a blinking indicator on D13
    pinMode(13, OUTPUT);
    while (true) {
      digitalWrite(13, !digitalRead(13));
      delay(200);
    }
  }

  display.clearDisplay();
  display.display();

  drawUI(0); // initial draw, volume unknown until first pot read
}

void loop() {
  handleButtons();
  handlePot();
}

void onButtonPressed(uint8_t index) {
  if (index == CAM_INDEX) {
    // Toggle CAM state and show ON/OFF
    camToggleOn = !camToggleOn;
    camShowingToggle = true;
  } else if (index == VC_INDEX) {
    // Toggle VC state and show ON/OFF
    vcToggleOn = !vcToggleOn;
    vcShowingToggle = true;
  } else {
    // Any other button press reverts CAM/VC labels back to default text
    camShowingToggle = false;
    vcShowingToggle = false;
  }
}

const char* getLabel(uint8_t index) {
  if (index == CAM_INDEX && camShowingToggle) {
    return camToggleOn ? "ON" : "OFF";
  }
  if (index == VC_INDEX && vcShowingToggle) {
    return vcToggleOn ? "ON" : "OFF";
  }
  return DEFAULT_LABELS[index];
}

void handleButtons() {
  for (uint8_t i = 0; i < 6; i++) {
    bool currentState = digitalRead(BUTTON_PINS[i]);

    if (currentState != lastState[i]) {
      lastDebounceTime[i] = millis();
    }

    if ((millis() - lastDebounceTime[i]) > DEBOUNCE_MS) {
      // Pressed = LOW because of INPUT_PULLUP wiring
      if (currentState == LOW && lastState[i] == HIGH) {
        Serial.print("KEY:");
        Serial.println(KEY_NUMBERS[i]);
        onButtonPressed(i);
        flashLabel(i);
      }
    }

    lastState[i] = currentState;
  }
}

void handlePot() {
  if (millis() - lastPotRead < POT_READ_INTERVAL) return;
  lastPotRead = millis();

  int raw = analogRead(POT_PIN);          // 0-1023
  int percent = map(raw, 0, 1023, 0, 100);

  // Small deadband so it doesn't jitter at the edges
  if (abs(percent - lastVolPercent) >= 1) {
    lastVolPercent = percent;
    Serial.print("VOL:");
    Serial.println(percent);
    drawUI(percent);
  }
}

void drawUI(int volPercent) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // 6 labels in a 3x2 grid (top area)
  display.setTextSize(1);
  int colW = SCREEN_WIDTH / 3;
  for (uint8_t i = 0; i < 6; i++) {
    int col = i % 3;
    int row = i / 3;
    int x = col * colW + 8;
    int y = row * 16 + 2;
    display.setCursor(x, y);
    display.print(getLabel(i));
  }

  // Divider line
  display.drawLine(0, 36, SCREEN_WIDTH, 36, SSD1306_WHITE);

  // Volume row: "-" on far left, speaker icon centered, "+" on far right
  int rowY = 44;     // top of this row's drawing area
  int rowH = 18;      // available height for icon

  // Minus sign (left)
  display.setTextSize(2);
  display.setCursor(4, rowY + 1);
  display.print("-");

  // Plus sign (right)
  display.setCursor(SCREEN_WIDTH - 16, rowY + 1);
  display.print("+");

  // Speaker icon (centered) - body + sound waves scaled by volume
  drawSpeakerIcon(SCREEN_WIDTH / 2, rowY + rowH / 2, volPercent);

  display.setTextSize(1);
  display.display();
}

void drawSpeakerIcon(int cx, int cy, int volPercent) {
  // Speaker body: small trapezoid+rectangle made of two filled triangles/rects
  int bodyW = 8;
  int bodyH = 10;
  int x = cx - 10;
  int y = cy - bodyH / 2;

  // Rectangle part of speaker body
  display.fillRect(x, y + 2, 4, bodyH - 4, SSD1306_WHITE);
  // Triangle (cone) part of speaker body
  display.fillTriangle(x + 4, y + 2, x + 4, y + bodyH - 2, x + bodyW, y, SSD1306_WHITE);
  display.fillTriangle(x + 4, y + bodyH - 2, x + bodyW, y, x + bodyW, y + bodyH, SSD1306_WHITE);

  // Sound wave arcs - number of arcs shown depends on volume level
  int numArcs = 0;
  if (volPercent > 0) numArcs = 1;
  if (volPercent > 33) numArcs = 2;
  if (volPercent > 66) numArcs = 3;

  int arcX = x + bodyW + 3;
  for (int i = 0; i < numArcs; i++) {
    int radius = 3 + i * 3;
    // Draw a partial arc using a circle clipped visually by only drawing right side
    for (int angle = -40; angle <= 40; angle += 10) {
      float rad = angle * 3.14159 / 180.0;
      int px = arcX + (int)(radius * cos(rad));
      int py = cy + (int)(radius * sin(rad));
      display.drawPixel(px, py, SSD1306_WHITE);
    }
  }

  if (volPercent == 0) {
    // Draw an "X" next to speaker to indicate muted
    display.drawLine(arcX, cy - 4, arcX + 6, cy + 4, SSD1306_WHITE);
    display.drawLine(arcX, cy + 4, arcX + 6, cy - 4, SSD1306_WHITE);
  }
}

void flashLabel(uint8_t index) {
  // Briefly invert the pressed button's label cell for visual feedback
  int col = index % 3;
  int row = index / 3;
  int colW = SCREEN_WIDTH / 3;
  int x = col * colW;
  int y = row * 16;

  display.fillRect(x, y, colW, 14, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(x + 8, y + 2);
  display.print(getLabel(index));
  display.display();
  delay(80);

  drawUI(lastVolPercent < 0 ? 0 : lastVolPercent);
}
