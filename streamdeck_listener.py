"""
Stream Deck PC-side listener.

Reads serial messages from the Arduino Nano and:
  - "KEY:13".."KEY:18"  -> simulates pressing F13-F18
  - "VOL:0".."VOL:100"  -> sets system volume to that percentage (Windows only)

Install dependencies first:
    pip install pyserial keyboard
    pip install pycaw comtypes      (Windows only, for volume control)

Run:
    python streamdeck_listener.py

If you don't know your COM port, run this script once - it will list
available ports and let you pick one.
"""

import sys
import time

import serial
import serial.tools.list_ports
import keyboard

# ---- Optional: Windows volume control ----
VOLUME_AVAILABLE = False
if sys.platform == "win32":
    try:
        from ctypes import cast, POINTER
        from comtypes import CLSCTX_ALL
        from pycaw.pycaw import AudioUtilities, IAudioEndpointVolume

        VOLUME_AVAILABLE = True
    except ImportError:
        print("[!] pycaw not installed - volume control disabled. "
              "Run: pip install pycaw comtypes")

BAUD_RATE = 115200

# Map key numbers from the Arduino to keyboard library key names
KEY_MAP = {
    13: "f13",
    14: "f14",
    15: "f15",
    16: "f16",
    17: "f17",
    18: "f18",
}


def get_volume_interface():
    if not VOLUME_AVAILABLE:
        return None
    devices = AudioUtilities.GetSpeakers()
    interface = devices.Activate(IAudioEndpointVolume._iid_, CLSCTX_ALL, None)
    return cast(interface, POINTER(IAudioEndpointVolume))


def set_system_volume(percent, volume_interface):
    if volume_interface is None:
        return
    percent = max(0, min(100, percent))
    # pycaw expects a scalar 0.0-1.0
    volume_interface.SetMasterVolumeLevelScalar(percent / 100.0, None)


def choose_port():
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        print("No serial ports found. Plug in the Nano and try again.")
        input("Press Enter to exit...")
        sys.exit(1)

    # Try to auto-detect an Arduino-like device (common USB-serial chip names)
    arduino_keywords = ("arduino", "ch340", "usb-serial", "usb serial", "wch")
    for p in ports:
        desc = (p.description or "").lower()
        if any(keyword in desc for keyword in arduino_keywords):
            print(f"Auto-detected Arduino on {p.device} ({p.description})")
            return p.device

    print("Available serial ports:")
    for i, p in enumerate(ports):
        print(f"  [{i}] {p.device} - {p.description}")

    if len(ports) == 1:
        print(f"Auto-selecting the only port found: {ports[0].device}")
        return ports[0].device

    choice = input("Select port number: ").strip()
    try:
        return ports[int(choice)].device
    except (ValueError, IndexError):
        print("Invalid choice.")
        input("Press Enter to exit...")
        sys.exit(1)


def main():
    port = choose_port()
    print(f"Connecting to {port} at {BAUD_RATE} baud...")

    try:
        ser = serial.Serial(port, BAUD_RATE, timeout=1)
    except serial.SerialException as e:
        print(f"Failed to open {port}: {e}")
        input("Press Enter to exit...")
        sys.exit(1)

    time.sleep(2)  # allow Arduino to reset after serial connection opens
    print("Connected. Listening for button presses and volume changes...")
    print("Press Ctrl+C to quit.\n")

    volume_interface = get_volume_interface()
    last_volume_print = -1

    try:
        while True:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if not line:
                continue

            if line.startswith("KEY:"):
                try:
                    key_num = int(line.split(":")[1])
                except (ValueError, IndexError):
                    continue

                key_name = KEY_MAP.get(key_num)
                if key_name:
                    keyboard.send(key_name)
                    print(f"[KEY] {key_name.upper()} pressed")

            elif line.startswith("VOL:"):
                try:
                    percent = int(line.split(":")[1])
                except (ValueError, IndexError):
                    continue

                if percent != last_volume_print:
                    last_volume_print = percent
                    set_system_volume(percent, volume_interface)
                    print(f"[VOL] {percent}%")

    except KeyboardInterrupt:
        print("\nShutting down.")
    finally:
        ser.close()
        input("Press Enter to exit...")


if __name__ == "__main__":
    main()
