# Arduino Nano Stream Deck DIY

A DIY 6-button stream deck with an OLED display and a volume knob, built on an
Arduino Nano.

## What's Included

| File | Purpose |
|---|---|
| `streamdeck.ino` | Arduino sketch — reads buttons/pot, drives the OLED, sends serial commands |
| `streamdeck_listener.py` | PC-side script — listens to serial, simulates keypresses, controls system volume |

## Hardware

- Arduino Nano
- 128x64 I2C OLED display (SSD1306, 4 pins: VCC, GND, SCL, SDA)
- 6x momentary push buttons
- 1x B5K potentiometer

## Wiring

| Component | Nano Pin |
|---|---|
| OLED SDA | A4 |
| OLED SCL | A5 |
| OLED VCC | 5V |
| OLED GND | GND |
| Button 1 (S1) | D2 → other leg to GND |
| Button 2 (S2) | D3 → other leg to GND |
| Button 3 (CAM) | D4 → other leg to GND |
| Button 4 (VC) | D5 → other leg to GND |
| Button 5 (SPT) | D6 → other leg to GND |
| Button 6 (TEL) | D7 → other leg to GND |
| Potentiometer outer legs | 5V and GND |
| Potentiometer wiper (middle) | A0 |

## Button Layout

| Pin | Key sent | OLED label | Behavior |
|---|---|---|---|
| D2 | F13 | S1 | static |
| D3 | F14 | S2 | static |
| D4 | F15 | CAM | toggles ON/OFF on press, reverts to "CAM" when another button is pressed |
| D5 | F16 | VC | toggles ON/OFF on press, reverts to "VC" when another button is pressed |
| D6 | F17 | SPT | static |
| D7 | F18 | TEL | static |

The potentiometer controls system volume, shown on the OLED as a speaker
icon between a "−" and "+", with the number of sound-wave arcs scaling with
volume level (0% shows a muted "X").

---

## Part 1: Flashing the Arduino

1. Open Arduino IDE.
2. Install required libraries via **Tools → Manage Libraries**:
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`
3. Select **Tools → Board → Arduino Nano**, and pick the correct processor
   (Old Bootloader / ATmega328P, depending on your specific Nano clone).
4. Select the correct **Port** under **Tools → Port**.
5. Open `streamdeck.ino`, click **Upload**.
6. Wire the hardware as described above. The OLED should light up showing
   the 6 labels and the volume row.

---

## Part 2: PC-Side Listener Script

The listener script runs on your computer, reads the serial messages coming
from the Nano, and converts them into actual keypresses and volume changes.

### Option A — Run from Python directly

1. Install Python 3 if you don't already have it: https://www.python.org/downloads/
2. Install dependencies:
   ```
   pip install pyserial keyboard pycaw comtypes
   ```
   (`pycaw` and `comtypes` are Windows-only and used for volume control. On
   Mac/Linux the script still runs — it just skips volume control.)
3. Run it:
   ```
   python streamdeck_listener.py
   ```
4. It will try to auto-detect the Nano's COM port. If it can't, it will list
   available ports and ask you to pick one.

### Option B — Package it as a Windows `.exe` (recommended for sharing)

This lets anyone run the listener without installing Python at all — just
double-click the `.exe`.

> **Note:** You must build the `.exe` on a Windows machine. PyInstaller
> builds for whatever OS it runs on, so building on Mac/Linux will not
> produce a Windows-compatible executable.

1. On a Windows PC, install the dependencies plus PyInstaller:
   ```
   pip install pyserial keyboard pycaw comtypes pyinstaller
   ```
2. In the folder containing `streamdeck_listener.py`, run:
   ```
   pyinstaller --onefile --console --name StreamDeckListener streamdeck_listener.py
   ```
3. PyInstaller creates several folders. Your finished executable is at:
   ```
   dist\StreamDeckListener.exe
   ```
4. Share just that one `.exe` file — it's fully self-contained.

**Flag reference:**
- `--onefile` — bundles everything into a single `.exe` file.
- `--console` — keeps a console window open so the user can see connection
  status, detected port, and button-press logs. Needed since the script can
  prompt for port selection if auto-detect fails.

### (Optional) Auto-start on Windows boot

To have the listener launch automatically whenever the PC starts:

1. Press `Win + R`, type `shell:startup`, press Enter. This opens the
   Startup folder.
2. Create a shortcut to `StreamDeckListener.exe` and drop it in that folder.

---

## Serial Protocol Reference

The Arduino sends plain text lines over serial at **115200 baud**:

| Message | Meaning |
|---|---|
| `KEY:13` … `KEY:18` | A button was pressed (momentary event) |
| `VOL:0` … `VOL:100` | Current potentiometer position as a volume percentage (sent only when it changes) |

This protocol is intentionally simple so you can extend it — e.g. add new
message types for additional buttons, RGB feedback, or app-specific
commands — without needing to change how the Arduino and PC connect.

## Troubleshooting

- **OLED stays blank**: double-check SDA/SCL wiring and that the OLED's I2C
  address matches `0x3C` (most common). Some boards use `0x3D` — if so,
  change the `display.begin(...)` call in `streamdeck.ino`.
- **Script can't find the Nano**: confirm the correct COM port appears in
  Windows Device Manager under "Ports (COM & LPT)". You may need a CH340
  driver if it's a clone Nano.
- **Keypresses don't register in an app**: confirm the target app has
  F13–F18 assigned as hotkeys in its own settings — these aren't standard
  keys most software listens for by default.
- **Volume doesn't change**: confirm `pycaw` and `comtypes` installed
  successfully; volume control only works on Windows.
