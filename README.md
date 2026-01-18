# XIAO ESP32S3 Sense MP3 Player

A magical little music box with organic LED breathing effects, built for the Seeed Studio XIAO ESP32S3 Sense.

![Version](https://img.shields.io/badge/version-1.0.2-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Platform](https://img.shields.io/badge/platform-ESP32--S3-orange)

## ✨ Features

- **9-track SD card playback** — Plays MP3 files from microSD card
- **Rotary encoder control** — Intuitive volume and playback control
- **Organic LED effects** — Breathing green/blue glow while playing
- **Track color identity** — Each track has a unique color for easy recognition
- **Low-power sleep** — LED off and minimal power draw when paused
- **Resume playback** — Wakes and continues from where you left off
- **Beautiful serial output** — Detailed, formatted logging for debugging
- **Servo record spin (easter egg)** — Optional continuous-rotation spin on GPIO41 while playing

## 🎮 Controls

| Action | Result |
|--------|--------|
| Rotate clockwise | Volume up |
| Rotate counter-clockwise | Volume down |
| Short press (<500ms) | Play / Pause |
| Long press (≥500ms) | Skip to next track |
| Hold press (≥5s) | Enable servo spin easter egg (resets on sleep) |
| Any interaction while sleeping | Wake and resume |

## 🌈 LED Feedback

| State | LED Behavior |
|-------|--------------|
| Playing | Breathing green/blue glow |
| Track change | Solid color for 1.5 seconds |
| Paused / Sleep | LED off |
| Error | Pulsing green |

### Track Colors

| Track | Color | Track | Color | Track | Color |
|-------|-------|-------|-------|-------|-------|
| 1 | 🟢 Green | 4 | 🌿 Mint | 7 | 🌊 Sea |
| 2 | 🔵 Blue | 5 | 🔹 Azure | 8 | 🌌 Sky |
| 3 | 🩵 Cyan | 6 | 🧊 Teal | 9 | ⚪ White |

## 🔧 Hardware Requirements

| Component | Description |
|-----------|-------------|
| XIAO ESP32S3 Sense | Seeed Studio board with camera expansion (includes SD slot) |
| MAX98357A | I2S DAC/Amplifier breakout |
| Speaker | 4Ω or 8Ω, 2-3W recommended |
| Rotary Encoder | KY-040 style with push button |
| Dual-color LED | Green + Blue LED, common cathode or common anode |
| Resistors | 2× 220Ω-470Ω for LED current limiting |
| MicroSD Card | FAT32 formatted |
| Continuous-rotation servo | Optional record-style spin effect (GPIO41) |

## 📌 Wiring

### Main Connections

```
XIAO ESP32S3 Sense          External Components
─────────────────           ────────────────────
D0  (GPIO1)  ───────────────> MAX98357A BCLK
D1  (GPIO2)  ───────────────> MAX98357A LRC
D2  (GPIO3)  ───────────────> MAX98357A DIN
D3  (GPIO4)  ───────────────> Encoder CLK
D4  (GPIO5)  ───────────────> Encoder DT
D5  (GPIO6)  ───────────────> Encoder SW
D6  (GPIO43) ──[220Ω]───────> LED Green
D7  (GPIO44) ──[220Ω]───────> LED Blue
D11 (GPIO41) ──────────────> Servo signal (continuous rotation)
3V3          ───────────────> Encoder VCC, MAX98357A VIN
GND          ───────────────> All grounds, LED common (cathode)
```

### MAX98357A I2S Amplifier — Complete Pin Reference

| Pin | Required? | Connect To | Purpose |
|-----|-----------|------------|---------|
| **VIN** | ✅ Yes | 3V3 or 5V | Power input. 5V = louder max output. |
| **GND** | ✅ Yes | GND | Ground reference. |
| **BCLK** | ✅ Yes | D0 (GPIO1) | I2S bit clock — timing for audio bits. |
| **LRC** | ✅ Yes | D1 (GPIO2) | I2S word select — left/right channel timing. |
| **DIN** | ✅ Yes | D2 (GPIO3) | I2S data — actual audio samples. |
| **GAIN** | ❌ Optional | See below | Sets amplifier gain (NOT volume control!). |
| **SD** | ❌ Optional | See below | Shutdown pin — enables/disables amp. |

Reminder: **SD must be floating or tied to VIN** (GND = mute), and **GAIN is optional** (NC/GND/VIN for boost).

#### GAIN Pin (Amplifier Boost Level)

The GAIN pin sets a **fixed hardware amplification level**. This is NOT the same as volume control!

| GAIN Connection | Boost Level | When to Use |
|-----------------|-------------|-------------|
| Floating (NC) | +9dB | Default. Good starting point. |
| Tied to GND | +12dB | If audio is too quiet at max volume. |
| Tied to VIN | +15dB | Maximum boost. May distort at high volume. |

**Volume control** is handled in software via the I2S audio library (0-21 levels). The rotary encoder adjusts this software volume. No hardware connection needed for volume!

#### SD Pin (Shutdown/Enable)

The SD pin enables or disables the amplifier output.

| SD Connection | Result |
|---------------|--------|
| Floating (NC) | Amplifier **enabled** (normal operation) |
| Tied to VIN | Amplifier **enabled** (same as floating) |
| Tied to GND | Amplifier **disabled** (muted, low power) |

For this project, leave SD floating or tie to VIN. You could optionally connect it to a spare GPIO for hardware mute control.

### Rotary Encoder (KY-040) — 5 Pins

The encoder provides **two functions**: rotation for volume control, and a built-in push button for playback control.

| Pin | Connect To | Purpose |
|-----|------------|---------|
| **CLK** | D3 (GPIO4) | Clock pulse — one pulse per detent (click) |
| **DT** | D4 (GPIO5) | Direction — phase relationship determines CW/CCW |
| **SW** | D5 (GPIO6) | Push button — built into encoder shaft, active LOW |
| **+** (VCC) | 3V3 | Power |
| **GND** | GND | Ground |

**Rotation** (CLK + DT): Rotate the knob to adjust volume. The code detects direction by comparing CLK and DT phase.

**Push Button** (SW): Press down on the encoder knob. Short press = play/pause, long press = next track.

### Dual-color LED

| Pin | Connect To | Notes |
|-----|------------|-------|
| Green | D6 (GPIO43) via 220Ω resistor | 220-470Ω is fine |
| Blue | D7 (GPIO44) via 220Ω resistor | Lower = brighter |
| Common | GND (cathode) or 3V3 (anode) | Set `LED_ACTIVE_LOW` in code |

### Servo (Continuous Rotation, Optional)

The servo is an easter egg: it is **off by default** and only spins when armed.

- Wire the servo signal to **D11 (GPIO41)** on the Sense expansion board (after cutting the J1–J2 trace).
- Hold the encoder button for **5 seconds** to arm the servo.
- The servo spins only while a track is **playing**.
- It pauses briefly during track changes/searches, then resumes.
- Entering sleep **disarms** the servo (you must re-arm it after wake).

### Pinout Reference

```
        XIAO ESP32S3 Sense (Front View)
        ┌─────────────────────────────┐
        │         [USB-C]             │
        │                             │
  D0 ───┤●                           ●├─── 5V
  D1 ───┤●                           ●├─── GND
  D2 ───┤●                           ●├─── 3V3
  D3 ───┤●                           ●├─── D10
  D4 ───┤●                           ●├─── D9
  D5 ───┤●                           ●├─── D8
  D6 ───┤●                           ●├─── D7
        │                             │
        │      [SD Card Slot]         │
        └─────────────────────────────┘
```

### Sense Expansion Board (D11/D12)

The Sense expansion board exposes two extra pins: **D11 (GPIO41)** and **D12 (GPIO42)**. By default they are connected to the onboard microphone. To repurpose them (for example, a servo PWM pin and an RGB channel), you must cut the **J1–J2** trace on the back of the expansion board (along the white line between the pads). Once cut, you can use **GPIO41** and **GPIO42** directly without affecting the SD card SPI pins. Note: these pins do **not** support ADC on the ESP32-S3.

## 💾 SD Card Setup

1. Format microSD card as **FAT32**
2. Copy MP3 files to root directory with these names:
   ```
   01.mp3
   02.mp3
   03.mp3
   04.mp3
   05.mp3
   06.mp3
   07.mp3
   08.mp3
   09.mp3
   ```
3. Insert card into the Sense expansion board slot

## 📦 Dependencies

Install via Arduino Library Manager:

| Library | Author | Search Term |
|---------|--------|-------------|
| ESP32-audioI2S | schreibfaul1 | "ESP32-audioI2S" |

## ⚙️ Arduino IDE Settings

> ⚠️ **IMPORTANT**: The default partition scheme is too small! You MUST change it or compilation will fail with "Sketch too big".

| Setting | Value | Notes |
|---------|-------|-------|
| Board | **XIAO_ESP32S3** | NOT "ESP32S3 Dev Module"! |
| USB CDC On Boot | Enabled | Required for Serial Monitor |
| Partition Scheme | **Huge APP (3MB No OTA/1MB SPIFFS)** | ⚠️ REQUIRED! |
| Flash Mode | QIO 80MHz | Default is fine |

### Why the partition scheme matters

The ESP32-audioI2S library pulls in WiFi/Network dependencies even though we don't use them. This makes the sketch ~1.8MB, but the default "4MB with spiffs" partition only allows 1.3MB for the app. Selecting "Huge APP" gives 3MB for the app, which is plenty.

## 🖥️ Serial Monitor Output

Connect at **115200 baud** to see formatted debug output:

```
╔═══════════════════════════════════════════════════════════╗
║                                                           ║
║     ♪ ♫  XIAO ESP32S3 Sense MP3 Player  ♫ ♪              ║
║                                                           ║
║          ~ A magical little music box ~                   ║
║                                                           ║
╚═══════════════════════════════════════════════════════════╝

[00:00.100] VER  Firmware v1.0.0 (2025-01-17)

[00:00.150] INIT Starting initialization...
[00:00.152] [OK]  GPIO configured
[00:00.155] [OK]  LED PWM initialized
[00:00.200] SD   Card: SDHC, Size: 7564 MB
[00:00.250] [OK]  SD card ready

────────────────────────────────────────────────────────────
[00:05.200] NOW  Playing Track 3 of 9
[00:05.201]      File:  /03.mp3
[00:05.202]      Color: Cyan
────────────────────────────────────────────────────────────

[00:10.500] VOL  Volume: [===============------] 15/21
```

## 🔧 Configuration

Key settings can be adjusted at the top of the sketch:

```cpp
// Audio
#define VOLUME_DEFAULT      15    // Initial volume (0-21)

// Timing
#define LONG_PRESS_MS       500   // Long press threshold
#define TRACK_COLOR_MS      1500  // Track color display duration
#define SERVO_ARM_HOLD_MS   5000  // Hold to enable servo easter egg
#define SERVO_RECORD_US     1620  // Servo pulse for record RPM (tune as needed)

// LED Animation
#define BREATH_SPEED        2     // Breathing animation speed
#define RAINBOW_INTERVAL_MS 50    // LED animation update rate
```

### Common Anode LED

If using a common anode LED, change this line:

```cpp
#define LED_ACTIVE_LOW      true   // Set to true for common anode
```

## 🐛 Troubleshooting

| Problem | Solution |
|---------|----------|
| SD card not detected | Ensure FAT32 format, try different card |
| No audio | Check BCLK/LRC/DIN wiring to MAX98357A |
| Audio crackling | Use separate power for amp, reduce volume |
| Encoder direction reversed | Swap CLK and DT wires |
| LED colors wrong | Check resistor connections, verify common cathode/anode setting |

## 📄 License

MIT License — see source file for full text.

## 🙏 Credits

- [ESP32-audioI2S](https://github.com/schreibfaul1/ESP32-audioI2S) by schreibfaul1
- [Seeed Studio](https://www.seeedstudio.com/) for the XIAO ESP32S3 Sense

---

*Made with ♥ for makers who love music*
