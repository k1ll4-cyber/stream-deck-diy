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

; --- CONFIGURE YOUR PORT HERE ---
SerialPort := "COM3" 

; Open serial connection with Arduino Nano (9600 Baud)
try {
    serial := FileOpen(SerialPort, "r+")
} catch {
    MsgBox("Failed to open " . SerialPort . ". Check connection or COM assignment.")
    ExitApp
}

SetTimer(ReadSerial, 10)

ReadSerial() {
    global serial
    if (serial && MsgWaitForLine(serial)) {
        line := Trim(serial.ReadLine())
        
        if (line == "SCENE1") {
            if !WinExist("ahk_exe obs64.exe")
                Run("C:\Program Files\obs-studio\bin\64bit\obs64.exe")
        }
        else if (line == "SCENE2") {
            Run("chrome.exe")
        }
        else if (line == "MUTE") {
            Send("{Volume_Mute}")
        }
        else if (line == "WEBCAM") {
            Send("{F16}") ; Assign this hotkey inside OBS to toggle camera source
        }
        else if (line == "TELEGRAM") {
            if telegramPID := ProcessExist("Telegram.exe")
                ProcessClose(telegramPID)
            else if FileExist(A_AppData . "\Telegram Desktop\Telegram.exe")
                Run(A_AppData . "\Telegram Desktop\Telegram.exe")
        }
        else if (line == "SPOTIFY") {
            if spotifyPID := ProcessExist("Spotify.exe")
                ProcessClose(spotifyPID)
            else if FileExist(A_AppData . "\Spotify\Spotify.exe")
                Run(A_AppData . "\Spotify\Spotify.exe")
        }
        else if (line == "VOL_UP") {
            Send("{Volume_Up}")
        }
        else if (line == "VOL_DOWN") {
            Send("{Volume_Down}")
        }
    }
}

MsgWaitForLine(file) {
    return file.Length > 0
}
