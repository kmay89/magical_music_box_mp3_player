# XIAO ESP32S3 Sense MP3 Player

A magical little music box with organic LED breathing effects, built for the Seeed Studio XIAO ESP32S3 Sense.

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Platform](https://img.shields.io/badge/platform-ESP32--S3-orange)

## ✨ Features

- **9-track SD card playback** — Plays MP3 files from microSD card
- **Rotary encoder control** — Intuitive volume and playback control
- **Organic LED effects** — Breathing rainbow glow while playing
- **Track color identity** — Each track has a unique color for easy recognition
- **Low-power sleep** — LED off and minimal power draw when paused
- **Resume playback** — Wakes and continues from where you left off
- **Beautiful serial output** — Detailed, formatted logging for debugging

## 🎮 Controls

| Action | Result |
|--------|--------|
| Rotate clockwise | Volume up |
| Rotate counter-clockwise | Volume down |
| Short press (<500ms) | Play / Pause |
| Long press (≥500ms) | Skip to next track |
| Any interaction while sleeping | Wake and resume |

## 🌈 LED Feedback

| State | LED Behavior |
|-------|--------------|
| Playing | Breathing rainbow glow |
| Track change | Solid color for 1.5 seconds |
| Paused / Sleep | LED off |
| Error | Pulsing red |

### Track Colors

| Track | Color | Track | Color | Track | Color |
|-------|-------|-------|-------|-------|-------|
| 1 | 🔴 Red | 4 | 🟢 Green | 7 | 🟣 Purple |
| 2 | 🟠 Orange | 5 | 🩵 Cyan | 8 | 💗 Pink |
| 3 | 🟡 Yellow | 6 | 🔵 Blue | 9 | ⚪ White |

## 🔧 Hardware Requirements

| Component | Description |
|-----------|-------------|
| XIAO ESP32S3 Sense | Seeed Studio board with camera expansion (includes SD slot) |
| MAX98357A | I2S DAC/Amplifier breakout |
| Speaker | 4Ω or 8Ω, 2-3W recommended |
| Rotary Encoder | KY-040 style with push button |
| RGB LED | Common cathode or common anode |
| Resistors | 3× 220Ω-470Ω for LED current limiting |
| MicroSD Card | FAT32 formatted |

## 📌 Wiring

```
XIAO ESP32S3 Sense          External Components
─────────────────           ────────────────────
D0  (GPIO1)  ───────────────> MAX98357A BCLK
D1  (GPIO2)  ───────────────> MAX98357A LRC
D2  (GPIO3)  ───────────────> MAX98357A DIN
D3  (GPIO4)  ───────────────> Encoder CLK
D4  (GPIO5)  ───────────────> Encoder DT
D5  (GPIO6)  ───────────────> Encoder SW
D8  (GPIO7)  ──[220Ω]───────> RGB LED Blue
D9  (GPIO8)  ──[220Ω]───────> RGB LED Green
D10 (GPIO9)  ──[220Ω]───────> RGB LED Red
3V3          ───────────────> Encoder VCC, MAX98357A VIN
GND          ───────────────> All grounds, LED common (cathode)
```

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

| Setting | Value |
|---------|-------|
| Board | XIAO_ESP32S3 |
| USB CDC On Boot | Enabled |
| Flash Mode | QIO 80MHz |
| Partition Scheme | Default 4MB with spiffs |

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
[00:05.202]      Color: Yellow
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

// LED Animation
#define BREATH_SPEED        2     // Breathing animation speed
#define RAINBOW_INTERVAL_MS 50    // Rainbow color cycle speed
```

### Common Anode LED

If using a common anode RGB LED, change this line:

```cpp
#define RGB_ACTIVE_LOW      true   // Set to true for common anode
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
