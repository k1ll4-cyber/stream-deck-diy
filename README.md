# stream-deck-diy
A Simple Stream Deck DIY.

A custom, open-source hardware macro pad built using an Arduino Nano, an I2C OLED display, a volume potentiometer, and 6 tactile push buttons. The hardware interfaces seamlessly with a background AutoHotkey (v2) script on Windows to trigger scene swaps in OBS, toggle system audio and webcams via custom keys, and launch or gracefully close applications like Telegram and Spotify with a single button press.

---

## 🚀 Features

*   **Dynamic OLED Dashboard:** Features a clean, single-row ticker tape UI. Pressing any button clean-swaps the header to display active states (`MIC: ON`, `CAM: OFF`, `STATUS: SPOTIFY`, etc.).
*   **Giant Volume Readout:** Displays a massive, responsive central numeric volume gauge ($0-100\%$) and matching horizontal progress slider.
*   **Intelligent App Toggling:** Multi-functional buttons that use AutoHotkey process tracking to open an application if it is closed, or gracefully terminate its background tasks if it is open.
*   **Zero External Components:** Utilizes the Arduino's internal `INPUT_PULLUP` network, completely removing the need for external pull-down resistors on the breadboard.

---

## 🛠️ Hardware Requirements & Pin Assignments

### Core Components
*   1x Arduino Nano (or compatible microcontroller)
*   1x 0.96" SSD1306 I2C OLED Display ($128 \times 64$)
*   1x 10k Linear Rotary Potentiometer
*   6x Tactile Push Buttons
*   1x Solderless Breadboard & Solid Core Jumper Wires

### GPIO Pin Configuration Matrix

| Component | Component Pin | Arduino Nano Pin | Destination / Function |
| :--- | :--- | :--- | :--- |
| **OLED Screen** | VCC / GND | 5V / GND Rails | System Power Bus |
| | SDA / SCL | **A4** / **A5** | Hardware I2C Interface |
| **Volume Dial** | Middle Pin (Wiper)| **A0** | Analog Voltage Sensor |
| **Button 1** | Input Pin | **D2** | OBS: Switch to Scene 1 |
| **Button 2** | Input Pin | **D3** | OBS: Switch to Scene 2 |
| **Button 3** | Input Pin | **D4** | Global Microphone Mute Toggle |
| **Button 4** | Input Pin | **D5** | Webcam View Matrix Toggle (`F16`) |
| **Button 5** | Input Pin | **D6** | Telegram Open/Close Toggle |
| **Button 6** | Input Pin | **D7** | Spotify Open/Close Toggle |

> **Wiring Note:** Connect the opposite diagonal leg of all 6 tactile push buttons directly to the breadboard's shared **Blue Negative Ground Rail (-)**.

---

## 💻 Software Setup

### 1. Arduino Firmware
1. Open the Arduino IDE.
2. Install the **Adafruit SSD1306** and **Adafruit GFX** libraries through the Library Manager.
3. Upload the controller firmware code (`.ino`) to your Arduino Nano.
4. Note down the active **COM Port** (e.g., COM3) assigned to your device by Windows via the Device Manager.

### 2. AutoHotkey (v2) Integration
Ensure you have **AutoHotkey v2.0+** installed on your Windows system. Create a script named `Stream.ahk` using the configuration snippet below, ensuring you modify the port settings to match your specific hardware setup:

```autohotkey
#Requires AutoHotkey v2.0
Persistent

arduinoPort := ""

Loop Reg, "HKLM\HARDWARE\DEVICEMAP\SERIALCOMM" {
    regValue := RegRead()
    if (regValue != "") {
        arduinoPort := "\\.\" . regValue
        break 
    }
}

if (arduinoPort == "") {
    MsgBox "No USB Stream Deck detected! Please make sure it is plugged in.", "Stream Deck Error", 48
    ExitApp
}

RunWait(A_ComSpec " /c mode " arduinoPort " BAUD=9600 PARITY=N DATA=8 STOP=1", , "Hide")

serial := FileOpen(arduinoPort, "r")

if !serial {
    MsgBox "Found device on " SubStr(arduinoPort, 5) " but couldn't open it. Is Serial Monitor still open?", "Port Blocked", 48
    ExitApp
}

TrayTip "Stream Deck Connected", "Auto-detected and listening on " SubStr(arduinoPort, 5), 1

SetTimer(ReadSerial, 10)

ReadSerial() {
    if (line := serial.ReadLine()) {
        line := Trim(line, "`r`n ")
        
        if (line == "")
            return
            
        if (line == "SCENE1")
            Send "{F13}"
        else if (line == "SCENE2")
            Send "{F14}"
        else if (line == "MUTE")
            Send "{F15}"
        else if (line == "WEBCAM")
            Send "{F16}"
        else if (line == "VOL_UP")
            Send "{Volume_Up}"
        else if (line == "VOL_DOWN")
            Send "{Volume_Down}"
        else if (line == "TELEGRAM")
        {
            telegramPath := A_AppData . "\Telegram Desktop\Telegram.exe"
            if FileExist(telegramPath)
                Run(telegramPath)
            else
                MsgBox("Telegram executable not found at:`n" . telegramPath)
        }
        else if (line == "SPOTIFY")
        {
            spotifyPath := A_AppData . "\Spotify\Spotify.exe"
            if FileExist(spotifyPath)
                Run(spotifyPath)
            else
                MsgBox("Spotify executable not found at:`n" . spotifyPath)
        }
    }
}
