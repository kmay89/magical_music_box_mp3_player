/**
 * ╔═══════════════════════════════════════════════════════════════════════════╗
 * ║                                                                           ║
 * ║   ♪ ♫  XIAO ESP32S3 Sense MP3 Player  ♫ ♪                                ║
 * ║                                                                           ║
 * ║   A magical little music box with organic LED breathing                   ║
 * ║                                                                           ║
 * ╚═══════════════════════════════════════════════════════════════════════════╝
 * 
 * @file      xiao_mp3_player.ino
 * @brief     SD card MP3 player with rotary encoder control and RGB LED feedback
 * @version   1.0.0
 * @date      2025-01-17
 * 
 * @hardware  Seeed Studio XIAO ESP32S3 Sense
 * @board     Select "XIAO_ESP32S3" in Arduino IDE
 * 
 * MIT License
 * 
 * Copyright (c) 2025
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * ─────────────────────────────────────────────────────────────────────────────
 * HARDWARE REQUIREMENTS
 * ─────────────────────────────────────────────────────────────────────────────
 * 
 *   Component                 Description
 *   ────────────────────────  ─────────────────────────────────────────────────
 *   XIAO ESP32S3 Sense        Seeed Studio board with camera expansion board
 *   MAX98357A                 I2S DAC/Amplifier module
 *   Speaker                   4Ω or 8Ω, 2-3W recommended
 *   Rotary Encoder            KY-040 style with push button (CLK, DT, SW)
 *   RGB LED                   Common cathode or common anode (3 pins + common)
 *   Resistors                 3x 220Ω-470Ω for LED current limiting
 *   MicroSD Card              FAT32 formatted, with MP3 files
 * 
 * ─────────────────────────────────────────────────────────────────────────────
 * WIRING DIAGRAM
 * ─────────────────────────────────────────────────────────────────────────────
 * 
 *   XIAO Pin    GPIO    Function              Connect To
 *   ──────────  ──────  ────────────────────  ──────────────────────────────
 *   D0          GPIO1   I2S Bit Clock         MAX98357A BCLK
 *   D1          GPIO2   I2S Word Select       MAX98357A LRC
 *   D2          GPIO3   I2S Data Out          MAX98357A DIN
 *   D3          GPIO4   Encoder Clock         Rotary Encoder CLK
 *   D4          GPIO5   Encoder Data          Rotary Encoder DT
 *   D5          GPIO6   Encoder Switch        Rotary Encoder SW
 *   D8          GPIO7   RGB Blue              Blue LED (via resistor)
 *   D9          GPIO8   RGB Green             Green LED (via resistor)
 *   D10         GPIO9   RGB Red               Red LED (via resistor)
 *   3V3         -       Power                 Encoder VCC, MAX98357A VIN
 *   GND         -       Ground                All component grounds
 * 
 *   Note: SD card uses internal routing on Sense expansion board (GPIO21 CS)
 * 
 * ─────────────────────────────────────────────────────────────────────────────
 * CONTROLS
 * ─────────────────────────────────────────────────────────────────────────────
 * 
 *   Action                    Result
 *   ────────────────────────  ─────────────────────────────────────────────────
 *   Rotate clockwise          Volume up
 *   Rotate counter-clockwise  Volume down
 *   Short press (<500ms)      Play/Pause (pause enters low-power sleep)
 *   Long press (≥500ms)       Skip to next track
 *   Any interaction (asleep)  Wake and resume playback
 * 
 * ─────────────────────────────────────────────────────────────────────────────
 * LED FEEDBACK
 * ─────────────────────────────────────────────────────────────────────────────
 * 
 *   State           LED Behavior
 *   ──────────────  ─────────────────────────────────────────────────────────
 *   Playing         Breathing rainbow glow (calming, organic animation)
 *   Track Change    Solid track color for 1.5 seconds (see track colors below)
 *   Paused/Sleep    LED off (low power mode)
 *   Error           Pulsing red
 * 
 *   Track Colors:
 *     Track 1: Red       Track 4: Green      Track 7: Purple
 *     Track 2: Orange    Track 5: Cyan       Track 8: Pink
 *     Track 3: Yellow    Track 6: Blue       Track 9: White
 * 
 * ─────────────────────────────────────────────────────────────────────────────
 * SD CARD SETUP
 * ─────────────────────────────────────────────────────────────────────────────
 * 
 *   1. Format microSD card as FAT32
 *   2. Copy MP3 files to root directory with names:
 *      01.mp3, 02.mp3, 03.mp3, 04.mp3, 05.mp3, 06.mp3, 07.mp3, 08.mp3, 09.mp3
 *   3. Insert card into Sense expansion board slot
 * 
 * ─────────────────────────────────────────────────────────────────────────────
 * DEPENDENCIES
 * ─────────────────────────────────────────────────────────────────────────────
 * 
 *   Library              Author        Install via Arduino Library Manager
 *   ───────────────────  ────────────  ─────────────────────────────────────
 *   ESP32-audioI2S       schreibfaul1  Search "ESP32-audioI2S"
 * 
 * ─────────────────────────────────────────────────────────────────────────────
 * ARDUINO IDE SETTINGS
 * ─────────────────────────────────────────────────────────────────────────────
 * 
 *   Board:              XIAO_ESP32S3
 *   USB CDC On Boot:    Enabled
 *   Flash Mode:         QIO 80MHz
 *   Partition Scheme:   Default 4MB with spiffs
 * 
 */

// ═══════════════════════════════════════════════════════════════════════════════
// INCLUDES
// ═══════════════════════════════════════════════════════════════════════════════

#include <Arduino.h>
#include <Audio.h>      // ESP32-audioI2S library
#include <SD.h>
#include <FS.h>

// ═══════════════════════════════════════════════════════════════════════════════
// VERSION INFO
// ═══════════════════════════════════════════════════════════════════════════════

#define FIRMWARE_VERSION    "1.0.0"
#define FIRMWARE_DATE       "2025-01-17"

// ═══════════════════════════════════════════════════════════════════════════════
// PIN DEFINITIONS - XIAO ESP32S3 Sense
// ═══════════════════════════════════════════════════════════════════════════════
//
// XIAO ESP32S3 Pinout Reference:
//
//   LEFT SIDE              RIGHT SIDE
//   ──────────             ──────────
//   D0  = GPIO1            5V
//   D1  = GPIO2            GND
//   D2  = GPIO3            3V3
//   D3  = GPIO4            D10 = GPIO9  (MOSI)
//   D4  = GPIO5  (SDA)     D9  = GPIO8  (MISO)
//   D5  = GPIO6  (SCL)     D8  = GPIO7  (SCK)
//   D6  = GPIO43 (TX)      D7  = GPIO44 (RX)
//
// Note: D8/D9/D10 are labeled as SPI but are available as GPIO when using
//       the Sense expansion board's SD card (uses internal SPI routing)

// RGB LED Configuration
// Set RGB_ACTIVE_LOW to true for common anode LED, false for common cathode
#define RGB_ACTIVE_LOW      false
#define PIN_LED_RED         9     // D10 = GPIO9
#define PIN_LED_GREEN       8     // D9  = GPIO8
#define PIN_LED_BLUE        7     // D8  = GPIO7

// I2S Audio (MAX98357A)
#define PIN_I2S_BCLK        1     // D0 = GPIO1  - Bit Clock
#define PIN_I2S_LRC         2     // D1 = GPIO2  - Word Select (LRC)
#define PIN_I2S_DOUT        3     // D2 = GPIO3  - Data Out

// Rotary Encoder
#define PIN_ENC_CLK         4     // D3 = GPIO4  - Clock (A)
#define PIN_ENC_DT          5     // D4 = GPIO5  - Data (B)
#define PIN_ENC_SW          6     // D5 = GPIO6  - Switch (active LOW)

// SD Card (Sense expansion board internal routing)
#define PIN_SD_CS           21    // Internal to expansion board

// ═══════════════════════════════════════════════════════════════════════════════
// CONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════════

// Serial
#define SERIAL_BAUD         115200

// Audio
#define NUM_TRACKS          9
#define VOLUME_DEFAULT      15
#define VOLUME_MIN          0
#define VOLUME_MAX          21
#define VOLUME_STEP         1

// Timing (milliseconds)
#define DEBOUNCE_BUTTON_MS  50
#define DEBOUNCE_ENCODER_MS 5
#define LONG_PRESS_MS       500
#define TRACK_COLOR_MS      1500
#define VOLUME_FLASH_MS     200
#define SLEEP_POLL_MS       50

// LED Animation
#define LED_PWM_FREQ        5000
#define LED_PWM_BITS        8     // 8-bit resolution (0-255)
#define BREATH_MIN          10    // Minimum brightness
#define BREATH_MAX          255   // Maximum brightness
#define BREATH_SPEED        2     // Animation speed multiplier
#define RAINBOW_INTERVAL_MS 50    // Time between hue steps

// ═══════════════════════════════════════════════════════════════════════════════
// TRACK DATA
// ═══════════════════════════════════════════════════════════════════════════════

static const char* const TRACK_FILES[NUM_TRACKS] = {
  "/01.mp3", "/02.mp3", "/03.mp3", "/04.mp3", "/05.mp3",
  "/06.mp3", "/07.mp3", "/08.mp3", "/09.mp3"
};

static const char* const TRACK_COLOR_NAMES[NUM_TRACKS] = {
  "Red", "Orange", "Yellow", "Green", "Cyan",
  "Blue", "Purple", "Pink", "White"
};

static const uint8_t TRACK_COLORS[NUM_TRACKS][3] = {
  {255,   0,   0},  // Track 1: Red
  {255, 127,   0},  // Track 2: Orange
  {255, 255,   0},  // Track 3: Yellow
  {  0, 255,   0},  // Track 4: Green
  {  0, 255, 255},  // Track 5: Cyan
  {  0,   0, 255},  // Track 6: Blue
  {127,   0, 255},  // Track 7: Purple
  {255,   0, 127},  // Track 8: Pink
  {255, 255, 255}   // Track 9: White
};

// ═══════════════════════════════════════════════════════════════════════════════
// TYPE DEFINITIONS
// ═══════════════════════════════════════════════════════════════════════════════

enum PlayerState {
  STATE_IDLE,
  STATE_PLAYING,
  STATE_PAUSED,
  STATE_ERROR
};

static const char* const STATE_NAMES[] = {
  "IDLE", "PLAYING", "PAUSED", "ERROR"
};

// ═══════════════════════════════════════════════════════════════════════════════
// GLOBAL STATE
// ═══════════════════════════════════════════════════════════════════════════════

static Audio audio;

// Player state
static PlayerState g_state       = STATE_IDLE;
static int         g_track       = 0;
static int         g_volume      = VOLUME_DEFAULT;
static bool        g_sdReady     = false;
static bool        g_sleeping    = false;

// Encoder state
static int          g_encLastCLK = HIGH;
static unsigned long g_encLastTime = 0;

// Button state
static unsigned long g_btnPressStart = 0;
static bool          g_btnPressed    = false;
static bool          g_btnHandled    = false;

// LED animation state
static float         g_breathPhase     = 0.0f;
static uint16_t      g_rainbowHue      = 0;
static unsigned long g_lastAnimTime    = 0;
static unsigned long g_trackColorStart = 0;
static bool          g_showingTrackColor = false;
static unsigned long g_volFlashStart   = 0;
static bool          g_showingVolFlash = false;

// Timing
static unsigned long g_startTime = 0;

// ═══════════════════════════════════════════════════════════════════════════════
// FUNCTION PROTOTYPES
// ═══════════════════════════════════════════════════════════════════════════════

// Setup
static void setupGPIO(void);
static void setupLED(void);
static void setupSD(void);
static void setupAudio(void);

// Core functions
static void handleEncoder(void);
static void handleButton(void);
static void updateLED(void);

// Playback control
static void playTrack(int index);
static void nextTrack(void);
static void togglePlayPause(void);
static void adjustVolume(int delta);

// Sleep/wake
static void enterSleep(void);
static void wakeUp(void);

// LED control
static void setLED(uint8_t r, uint8_t g, uint8_t b);
static void showTrackColor(int index);
static void showVolumeFlash(void);
static void breathingRainbow(void);
static void hsvToRgb(uint16_t h, uint8_t s, uint8_t v, uint8_t* r, uint8_t* g, uint8_t* b);

// Utilities
static bool fileExists(const char* path);
static void scanTracks(void);

// Serial output
static void printHeader(void);
static void printWelcome(void);
static void printDivider(void);
static void printDoubleDivider(void);
static void printTimestamp(void);
static void printStatus(const char* tag, const char* msg);
static void printStatusF(const char* tag, const char* fmt, ...);
static void printVolumeBar(int vol);
static void printTrackInfo(int index);
static void printStateChange(PlayerState from, PlayerState to);
static void printSleepEnter(void);
static void printWakeUp(void);
static void printError(const char* msg);
static void printSuccess(const char* msg);
static void printSDInfo(uint8_t type, uint64_t size);
static void printTrackList(const bool* found);

// ═══════════════════════════════════════════════════════════════════════════════
// SERIAL OUTPUT FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════

static void printDivider(void) {
  Serial.println(F("────────────────────────────────────────────────────────────"));
}

static void printDoubleDivider(void) {
  Serial.println(F("════════════════════════════════════════════════════════════"));
}

static void printTimestamp(void) {
  unsigned long elapsed = millis() - g_startTime;
  unsigned long secs = elapsed / 1000;
  unsigned long mins = secs / 60;
  secs %= 60;
  elapsed %= 1000;
  Serial.printf("[%02lu:%02lu.%03lu] ", mins, secs, elapsed);
}

static void printHeader(void) {
  Serial.println();
  Serial.println(F("╔═══════════════════════════════════════════════════════════╗"));
  Serial.println(F("║                                                           ║"));
  Serial.println(F("║     ♪ ♫  XIAO ESP32S3 Sense MP3 Player  ♫ ♪              ║"));
  Serial.println(F("║                                                           ║"));
  Serial.println(F("║          ~ A magical little music box ~                   ║"));
  Serial.println(F("║                                                           ║"));
  Serial.println(F("╚═══════════════════════════════════════════════════════════╝"));
  Serial.println();
  printTimestamp();
  Serial.printf("VER  Firmware v%s (%s)\n", FIRMWARE_VERSION, FIRMWARE_DATE);
  Serial.println();
}

static void printStatus(const char* tag, const char* msg) {
  printTimestamp();
  Serial.printf("%-4s %s\n", tag, msg);
}

static void printStatusF(const char* tag, const char* fmt, ...) {
  printTimestamp();
  Serial.printf("%-4s ", tag);
  
  char buf[128];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  Serial.println(buf);
}

static void printVolumeBar(int vol) {
  printTimestamp();
  Serial.print(F("VOL  Volume: ["));
  
  const int barWidth = 21;
  int filled = (vol * barWidth) / VOLUME_MAX;
  
  for (int i = 0; i < barWidth; i++) {
    Serial.print(i < filled ? '=' : '-');
  }
  
  Serial.printf("] %d/%d\n", vol, VOLUME_MAX);
}

static void printTrackInfo(int index) {
  Serial.println();
  printDivider();
  printTimestamp();
  Serial.printf("NOW  Playing Track %d of %d\n", index + 1, NUM_TRACKS);
  printTimestamp();
  Serial.printf("     File:  %s\n", TRACK_FILES[index]);
  printTimestamp();
  Serial.printf("     Color: %s\n", TRACK_COLOR_NAMES[index]);
  printDivider();
  Serial.println();
}

static void printStateChange(PlayerState from, PlayerState to) {
  printTimestamp();
  Serial.printf(">>>  State: %s --> %s\n", STATE_NAMES[from], STATE_NAMES[to]);
}

static void printWelcome(void) {
  Serial.println();
  Serial.println(F("┌───────────────────────────────────────────────────────────┐"));
  Serial.println(F("│  [OK] System initialized successfully!                    │"));
  Serial.println(F("│                                                           │"));
  Serial.println(F("│  CONTROLS:                                                │"));
  Serial.println(F("│    Rotate encoder ────────> Volume Up/Down                │"));
  Serial.println(F("│    Short press (<500ms) ──> Play / Pause                  │"));
  Serial.println(F("│    Long press (≥500ms) ───> Next Track                    │"));
  Serial.println(F("│                                                           │"));
  Serial.println(F("│  LED FEEDBACK:                                            │"));
  Serial.println(F("│    Playing ───────────────> Breathing rainbow glow        │"));
  Serial.println(F("│    Track change ──────────> Track's unique color          │"));
  Serial.println(F("│    Paused/Sleep ──────────> LED off (low power)           │"));
  Serial.println(F("│                                                           │"));
  Serial.println(F("│  WIRING CHECK:                                            │"));
  Serial.println(F("│    D0/GPIO1  --> MAX98357A BCLK                           │"));
  Serial.println(F("│    D1/GPIO2  --> MAX98357A LRC                            │"));
  Serial.println(F("│    D2/GPIO3  --> MAX98357A DIN                            │"));
  Serial.println(F("│    D3/GPIO4  --> Encoder CLK                              │"));
  Serial.println(F("│    D4/GPIO5  --> Encoder DT                               │"));
  Serial.println(F("│    D5/GPIO6  --> Encoder SW                               │"));
  Serial.println(F("│    D8/GPIO7  --> RGB Blue    (via resistor)               │"));
  Serial.println(F("│    D9/GPIO8  --> RGB Green   (via resistor)               │"));
  Serial.println(F("│    D10/GPIO9 --> RGB Red     (via resistor)               │"));
  Serial.println(F("│                                                           │"));
  Serial.println(F("│  Press the encoder to begin...                            │"));
  Serial.println(F("└───────────────────────────────────────────────────────────┘"));
  Serial.println();
}

static void printSleepEnter(void) {
  Serial.println();
  printTimestamp();
  Serial.println(F("zzz  Entering sleep mode... (LED off, low power)"));
  printTimestamp();
  Serial.println(F("     Press or rotate encoder to wake"));
  Serial.println();
}

static void printWakeUp(void) {
  Serial.println();
  printTimestamp();
  Serial.println(F("***  Waking up!"));
}

static void printError(const char* msg) {
  Serial.println();
  printTimestamp();
  Serial.printf("[ERR] %s\n", msg);
  Serial.println();
}

static void printSuccess(const char* msg) {
  printTimestamp();
  Serial.printf("[OK]  %s\n", msg);
}

static void printSDInfo(uint8_t type, uint64_t size) {
  const char* typeStr = "UNKNOWN";
  switch (type) {
    case CARD_MMC:  typeStr = "MMC";  break;
    case CARD_SD:   typeStr = "SD";   break;
    case CARD_SDHC: typeStr = "SDHC"; break;
  }
  printTimestamp();
  Serial.printf("SD   Card: %s, Size: %llu MB\n", typeStr, size / (1024 * 1024));
}

static void printTrackList(const bool* found) {
  Serial.println();
  printTimestamp();
  Serial.println(F("SCAN Track Library:"));
  
  int count = 0;
  for (int i = 0; i < NUM_TRACKS; i++) {
    printTimestamp();
    if (found[i]) {
      Serial.printf("     [%d] %-8s %s  [FOUND]\n", 
                    i + 1, TRACK_COLOR_NAMES[i], TRACK_FILES[i]);
      count++;
    } else {
      Serial.printf("     [%d] %-8s %s  [MISSING]\n", 
                    i + 1, TRACK_COLOR_NAMES[i], TRACK_FILES[i]);
    }
  }
  
  Serial.println();
  printTimestamp();
  Serial.printf("STAT Found %d of %d tracks\n", count, NUM_TRACKS);
  
  if (count < NUM_TRACKS) {
    printTimestamp();
    Serial.println(F("     Missing tracks will be skipped during playback"));
  }
  Serial.println();
}

// ═══════════════════════════════════════════════════════════════════════════════
// LED CONTROL FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════

static void setLED(uint8_t r, uint8_t g, uint8_t b) {
  if (RGB_ACTIVE_LOW) {
    // Common anode: invert values
    ledcWrite(PIN_LED_RED,   255 - r);
    ledcWrite(PIN_LED_GREEN, 255 - g);
    ledcWrite(PIN_LED_BLUE,  255 - b);
  } else {
    // Common cathode: direct values
    ledcWrite(PIN_LED_RED,   r);
    ledcWrite(PIN_LED_GREEN, g);
    ledcWrite(PIN_LED_BLUE,  b);
  }
}

static void hsvToRgb(uint16_t h, uint8_t s, uint8_t v, uint8_t* r, uint8_t* g, uint8_t* b) {
  if (s == 0) {
    *r = *g = *b = v;
    return;
  }
  
  uint8_t region = h / 60;
  uint8_t remainder = (h - (region * 60)) * 255 / 60;
  uint8_t p = (v * (255 - s)) >> 8;
  uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
  uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
  
  switch (region) {
    case 0:  *r = v; *g = t; *b = p; break;
    case 1:  *r = q; *g = v; *b = p; break;
    case 2:  *r = p; *g = v; *b = t; break;
    case 3:  *r = p; *g = q; *b = v; break;
    case 4:  *r = t; *g = p; *b = v; break;
    default: *r = v; *g = p; *b = q; break;
  }
}

static void showTrackColor(int index) {
  if (index < 0 || index >= NUM_TRACKS) return;
  
  setLED(TRACK_COLORS[index][0], 
         TRACK_COLORS[index][1], 
         TRACK_COLORS[index][2]);
  
  g_showingTrackColor = true;
  g_trackColorStart = millis();
}

static void showVolumeFlash(void) {
  setLED(100, 100, 100);  // Brief white flash
  g_showingVolFlash = true;
  g_volFlashStart = millis();
}

static void breathingRainbow(void) {
  // Breathing effect using sine wave
  g_breathPhase += 0.02f * BREATH_SPEED;
  if (g_breathPhase > TWO_PI) {
    g_breathPhase -= TWO_PI;
  }
  
  float breathVal = (sinf(g_breathPhase) + 1.0f) / 2.0f;
  uint8_t brightness = BREATH_MIN + (uint8_t)(breathVal * (BREATH_MAX - BREATH_MIN));
  
  // Slowly cycle through hues
  g_rainbowHue = (g_rainbowHue + 1) % 360;
  
  uint8_t r, g, b;
  hsvToRgb(g_rainbowHue, 255, brightness, &r, &g, &b);
  setLED(r, g, b);
}

static void updateLED(void) {
  unsigned long now = millis();
  
  // Volume flash takes priority (brief feedback)
  if (g_showingVolFlash) {
    if (now - g_volFlashStart >= VOLUME_FLASH_MS) {
      g_showingVolFlash = false;
    } else {
      return;
    }
  }
  
  // Track color display
  if (g_showingTrackColor) {
    if (now - g_trackColorStart >= TRACK_COLOR_MS) {
      g_showingTrackColor = false;
      g_breathPhase = 0.0f;  // Reset for smooth transition
    } else {
      return;
    }
  }
  
  // State-based animation
  switch (g_state) {
    case STATE_PLAYING:
      if (now - g_lastAnimTime >= RAINBOW_INTERVAL_MS) {
        g_lastAnimTime = now;
        breathingRainbow();
      }
      break;
      
    case STATE_PAUSED:
    case STATE_IDLE:
      setLED(0, 0, 30);  // Dim blue standby
      break;
      
    case STATE_ERROR: {
      g_breathPhase += 0.1f;
      if (g_breathPhase > TWO_PI) g_breathPhase -= TWO_PI;
      uint8_t brightness = 50 + (uint8_t)((sinf(g_breathPhase) + 1.0f) * 100.0f);
      setLED(brightness, 0, 0);  // Pulsing red
      break;
    }
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// SLEEP / WAKE FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════

static void enterSleep(void) {
  g_sleeping = true;
  setLED(0, 0, 0);
  printSleepEnter();
}

static void wakeUp(void) {
  printWakeUp();
  g_sleeping = false;
  
  delay(100);  // Debounce
  g_encLastCLK = digitalRead(PIN_ENC_CLK);
  
  // Clear pending button state
  g_btnPressed = false;
  g_btnHandled = false;
  
  if (g_state == STATE_PAUSED) {
    audio.pauseResume();
    PlayerState oldState = g_state;
    g_state = STATE_PLAYING;
    printStateChange(oldState, g_state);
    printStatus("PLAY", "Resumed playback");
  } else if (g_state == STATE_IDLE) {
    playTrack(g_track);
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// SD CARD FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════

static bool fileExists(const char* path) {
  File f = SD.open(path);
  if (f) {
    bool isFile = !f.isDirectory();
    f.close();
    return isFile;
  }
  return false;
}

static void setupSD(void) {
  printStatus("SD", "Initializing SD card...");
  
  if (!SD.begin(PIN_SD_CS)) {
    printError("SD card mount failed!");
    printTimestamp();
    Serial.println(F("     Check: Is card inserted? Is it FAT32 formatted?"));
    g_sdReady = false;
    return;
  }
  
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    printError("No SD card detected!");
    g_sdReady = false;
    return;
  }
  
  printSDInfo(cardType, SD.cardSize());
  g_sdReady = true;
  printSuccess("SD card ready");
}

static void scanTracks(void) {
  printStatus("SCAN", "Scanning for tracks...");
  
  bool found[NUM_TRACKS];
  for (int i = 0; i < NUM_TRACKS; i++) {
    found[i] = fileExists(TRACK_FILES[i]);
  }
  
  printTrackList(found);
}

// ═══════════════════════════════════════════════════════════════════════════════
// AUDIO FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════

static void setupAudio(void) {
  printStatus("AUD", "Initializing audio system...");
  
  audio.setPinout(PIN_I2S_BCLK, PIN_I2S_LRC, PIN_I2S_DOUT);
  audio.setVolume(g_volume);
  
  printSuccess("Audio system ready");
  printVolumeBar(g_volume);
}

static void playTrack(int index) {
  if (!g_sdReady) {
    printError("SD card not ready!");
    g_state = STATE_ERROR;
    return;
  }
  
  // Validate and wrap index
  if (index < 0 || index >= NUM_TRACKS) {
    index = 0;
  }
  
  g_track = index;
  showTrackColor(g_track);
  
  const char* filename = TRACK_FILES[g_track];
  
  // Find next available track if current doesn't exist
  if (!fileExists(filename)) {
    printStatusF("WARN", "Track %d not found, searching...", g_track + 1);
    
    int attempts = 0;
    while (!fileExists(TRACK_FILES[g_track]) && attempts < NUM_TRACKS) {
      g_track = (g_track + 1) % NUM_TRACKS;
      attempts++;
    }
    
    if (attempts >= NUM_TRACKS) {
      printError("No playable tracks found on SD card!");
      g_state = STATE_ERROR;
      setLED(255, 0, 0);
      return;
    }
    
    filename = TRACK_FILES[g_track];
    showTrackColor(g_track);
  }
  
  if (audio.connecttoFS(SD, filename)) {
    PlayerState oldState = g_state;
    g_state = STATE_PLAYING;
    printStateChange(oldState, g_state);
    printTrackInfo(g_track);
  } else {
    printError("Failed to start playback!");
    g_state = STATE_ERROR;
    setLED(255, 0, 0);
  }
}

static void nextTrack(void) {
  printStatus("SKIP", "Skipping to next track...");
  audio.stopSong();
  playTrack((g_track + 1) % NUM_TRACKS);
}

static void togglePlayPause(void) {
  switch (g_state) {
    case STATE_IDLE:
      printStatus("PLAY", "Starting playback...");
      playTrack(g_track);
      break;
      
    case STATE_PLAYING: {
      audio.pauseResume();
      PlayerState oldState = g_state;
      g_state = STATE_PAUSED;
      printStateChange(oldState, g_state);
      printStatus("PAUS", "Paused");
      enterSleep();
      break;
    }
      
    case STATE_PAUSED: {
      audio.pauseResume();
      PlayerState oldState = g_state;
      g_state = STATE_PLAYING;
      printStateChange(oldState, g_state);
      printStatus("PLAY", "Resumed");
      break;
    }
      
    case STATE_ERROR:
      printStatus("RTRY", "Attempting recovery...");
      playTrack(g_track);
      break;
  }
}

static void adjustVolume(int delta) {
  int newVol = constrain(g_volume + delta, VOLUME_MIN, VOLUME_MAX);
  
  if (newVol != g_volume) {
    g_volume = newVol;
    audio.setVolume(g_volume);
    showVolumeFlash();
    printVolumeBar(g_volume);
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// INPUT HANDLING FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════

static void handleEncoder(void) {
  int clkState = digitalRead(PIN_ENC_CLK);
  
  if (millis() - g_encLastTime < DEBOUNCE_ENCODER_MS) {
    return;
  }
  
  if (clkState != g_encLastCLK && clkState == LOW) {
    g_encLastTime = millis();
    int delta = (digitalRead(PIN_ENC_DT) != clkState) ? VOLUME_STEP : -VOLUME_STEP;
    adjustVolume(delta);
  }
  
  g_encLastCLK = clkState;
}

static void handleButton(void) {
  bool pressed = !digitalRead(PIN_ENC_SW);  // Active LOW
  unsigned long now = millis();
  
  // Button just pressed
  if (pressed && !g_btnPressed) {
    g_btnPressStart = now;
    g_btnPressed = true;
    g_btnHandled = false;
  }
  
  // Button being held - check for long press
  if (pressed && g_btnPressed && !g_btnHandled) {
    if (now - g_btnPressStart >= LONG_PRESS_MS) {
      printStatus("BTN", "Long press detected");
      nextTrack();
      g_btnHandled = true;
    }
  }
  
  // Button released
  if (!pressed && g_btnPressed) {
    if (!g_btnHandled && (now - g_btnPressStart) >= DEBOUNCE_BUTTON_MS) {
      printStatus("BTN", "Short press detected");
      togglePlayPause();
    }
    g_btnPressed = false;
    g_btnHandled = false;
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// SETUP FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════

static void setupGPIO(void) {
  // LED pins
  pinMode(PIN_LED_RED,   OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE,  OUTPUT);
  
  // Encoder pins with internal pull-ups
  pinMode(PIN_ENC_CLK, INPUT_PULLUP);
  pinMode(PIN_ENC_DT,  INPUT_PULLUP);
  pinMode(PIN_ENC_SW,  INPUT_PULLUP);
  
  g_encLastCLK = digitalRead(PIN_ENC_CLK);
  
  printSuccess("GPIO configured");
}

static void setupLED(void) {
  ledcAttach(PIN_LED_RED,   LED_PWM_FREQ, LED_PWM_BITS);
  ledcAttach(PIN_LED_GREEN, LED_PWM_FREQ, LED_PWM_BITS);
  ledcAttach(PIN_LED_BLUE,  LED_PWM_FREQ, LED_PWM_BITS);
  
  setLED(128, 0, 128);  // Purple during initialization
  
  printSuccess("LED PWM initialized");
}

// ═══════════════════════════════════════════════════════════════════════════════
// ARDUINO ENTRY POINTS
// ═══════════════════════════════════════════════════════════════════════════════

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1000);  // Allow serial to initialize
  
  g_startTime = millis();
  
  printHeader();
  printStatus("INIT", "Starting initialization...");
  
  setupGPIO();
  setupLED();
  setupSD();
  
  if (g_sdReady) {
    setupAudio();
    scanTracks();
    
    g_track = 0;
    g_state = STATE_IDLE;
    
    printDoubleDivider();
    printWelcome();
    printDoubleDivider();
    
    enterSleep();
  } else {
    g_state = STATE_ERROR;
    setLED(255, 0, 0);
    printError("Initialization failed - check SD card");
  }
}

void loop() {
  // Sleep mode: minimal processing
  if (g_sleeping) {
    bool buttonWake = !digitalRead(PIN_ENC_SW);
    bool encoderWake = digitalRead(PIN_ENC_CLK) != g_encLastCLK;
    
    if (buttonWake || encoderWake) {
      wakeUp();
    }
    
    delay(SLEEP_POLL_MS);
    return;
  }
  
  // Normal operation
  audio.loop();
  handleEncoder();
  handleButton();
  updateLED();
}

// ═══════════════════════════════════════════════════════════════════════════════
// AUDIO LIBRARY CALLBACKS
// ═══════════════════════════════════════════════════════════════════════════════

void audio_eof_mp3(const char* info) {
  printStatus("END", "Track finished");
  nextTrack();
}

void audio_id3data(const char* info) {
  printStatusF("ID3", "%s", info);
}

void audio_info(const char* info) {
  // Filter to show only useful information
  String s(info);
  if (s.indexOf("BitRate") >= 0 || s.indexOf("SampleRate") >= 0) {
    printStatusF("INFO", "%s", info);
  }
}

void audio_error(const char* info) {
  printStatusF("ERR", "Audio: %s", info);
  g_state = STATE_ERROR;
  setLED(255, 0, 0);
}

void audio_bitrate(const char* info) {
  printStatusF("RATE", "Bitrate: %s", info);
}

// Unused callbacks - required by library but not needed for SD playback
void audio_commercial(const char* info)       { (void)info; }
void audio_icyurl(const char* info)           { (void)info; }
void audio_lasthost(const char* info)         { (void)info; }
void audio_showstation(const char* info)      { (void)info; }
void audio_showstreamtitle(const char* info)  { (void)info; }
