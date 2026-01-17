/**
 * ╔═══════════════════════════════════════════════════════════════════════════╗
 * ║     ♪ ♫  XIAO ESP32S3 Sense MP3 Player  ♫ ♪    v1.0.2                    ║
 * ╚═══════════════════════════════════════════════════════════════════════════╝
 * 
 * @file      xiao_mp3_player.ino
 * @brief     SD card MP3 player with rotary encoder and RGB LED feedback
 * @version   1.0.2
 * @date      2025-01-17
 * @hardware  Seeed Studio XIAO ESP32S3 Sense
 * @license   MIT
 * 
 * ARDUINO IDE SETTINGS (IMPORTANT - READ CAREFULLY):
 * ──────────────────────────────────────────────────────────────────────────────
 *   Board:             "XIAO_ESP32S3" (NOT "ESP32S3 Dev Module"!)
 *   USB CDC On Boot:   "Enabled"
 *   Partition Scheme:  "Huge APP (3MB No OTA/1MB SPIFFS)"  <-- REQUIRED!
 *   Flash Mode:        "QIO 80MHz"
 * 
 *   The default partition is too small for the audio library dependencies.
 *   You MUST select "Huge APP" or compilation will fail with "Sketch too big".
 * 
 * DEPENDENCIES (Install via Arduino Library Manager):
 * ──────────────────────────────────────────────────────────────────────────────
 *   ESP32-audioI2S by schreibfaul1 — Search "ESP32-audioI2S"
 * 
 * WIRING SUMMARY:
 * ──────────────────────────────────────────────────────────────────────────────
 *   XIAO Pin   GPIO   Function              Connect To
 *   ─────────  ─────  ────────────────────  ──────────────────────────────────
 *   D0         1      I2S Bit Clock         MAX98357A BCLK
 *   D1         2      I2S Word Select       MAX98357A LRC  
 *   D2         3      I2S Data Out          MAX98357A DIN
 *   D3         4      Encoder CLK           Rotary Encoder CLK (rotation pulse)
 *   D4         5      Encoder DT            Rotary Encoder DT (rotation direction)
 *   D5         6      Encoder SW            Rotary Encoder SW (push button)
 *   D8         7      LED Blue              RGB LED Blue + 220Ω resistor
 *   D9         8      LED Green             RGB LED Green + 220Ω resistor
 *   D10        9      LED Red               RGB LED Red + 220Ω resistor
 *   3V3        -      3.3V Power            Encoder +/VCC, MAX98357A VIN
 *   GND        -      Ground                All grounds, LED cathode
 * 
 * ROTARY ENCODER (KY-040 style with 5 pins):
 * ──────────────────────────────────────────────────────────────────────────────
 *   The rotary encoder provides TWO controls in one component:
 *     1. ROTATION (CLK + DT pins) — Turn knob to adjust volume
 *     2. PUSH BUTTON (SW pin) — Press knob for play/pause/skip
 *   
 *   Encoder Pin    XIAO Pin    Function
 *   ───────────    ─────────   ────────────────────────────────────────────────
 *   CLK            D3/GPIO4    Clock pulse — one pulse per detent (click)
 *   DT             D4/GPIO5    Direction — phase vs CLK determines CW/CCW
 *   SW             D5/GPIO6    Switch — push down on knob (active LOW)
 *   + (VCC)        3V3         Power for encoder
 *   GND            GND         Ground
 * 
 * MAX98357A PIN REFERENCE:
 * ──────────────────────────────────────────────────────────────────────────────
 *   Pin     Required?  Connect To     Purpose
 *   ──────  ─────────  ───────────    ─────────────────────────────────────────
 *   VIN     YES        3V3 or 5V      Power input. 5V = louder output.
 *   GND     YES        GND            Ground reference.
 *   BCLK    YES        D0 (GPIO1)     I2S bit clock from ESP32.
 *   LRC     YES        D1 (GPIO2)     I2S left/right clock (word select).
 *   DIN     YES        D2 (GPIO3)     I2S audio data from ESP32.
 *   
 *   GAIN    OPTIONAL   See below      Sets FIXED amplifier gain (not volume!):
 *                                       - Floating (NC): 9dB gain (default)
 *                                       - Tied to GND:   12dB gain
 *                                       - Tied to VIN:   15dB gain (loudest)
 *                                     NOTE: This does NOT control volume!
 *                                     Volume is controlled via software (I2S).
 *                                     GAIN just sets how much the amp boosts
 *                                     the signal. Higher = louder at same volume.
 *   
 *   SD      OPTIONAL   See below      Shutdown/Enable pin:
 *                                       - Floating (NC): Amp is ENABLED
 *                                       - Tied to GND:   Amp is DISABLED (muted)
 *                                       - Tied to VIN:   Amp is ENABLED
 *                                     Leave floating or tie to VIN for normal use.
 *                                     Could connect to a GPIO for hardware mute.
 * 
 * VOLUME CONTROL:
 * ──────────────────────────────────────────────────────────────────────────────
 *   Volume is controlled in SOFTWARE via the audio library (0-21 levels).
 *   The rotary encoder adjusts this software volume.
 *   No hardware connection needed for volume - just BCLK, LRC, DIN, VIN, GND.
 *   
 *   The GAIN pin only sets a fixed amplifier boost level. If audio is too
 *   quiet even at max software volume, tie GAIN to VIN for +15dB boost.
 *   If audio distorts at high volume, tie GAIN to GND for +12dB or leave
 *   floating for +9dB.
 * 
 * SD CARD:
 * ──────────────────────────────────────────────────────────────────────────────
 *   Uses built-in slot on Sense expansion board (no wiring needed).
 *   Format: FAT32. Files: 01.mp3, 02.mp3, ... 09.mp3 in root directory.
 */

#include <Arduino.h>
#include <Audio.h>
#include <SD.h>
#include <FS.h>

// ═══════════════════════════════════════════════════════════════════════════════
// VERSION
// ═══════════════════════════════════════════════════════════════════════════════

#define FIRMWARE_VERSION    "1.0.2"
#define FIRMWARE_DATE       "2025-01-17"

// ═══════════════════════════════════════════════════════════════════════════════
// PIN DEFINITIONS - XIAO ESP32S3 Sense
// ═══════════════════════════════════════════════════════════════════════════════
//
// XIAO ESP32S3 Pinout (active pins marked with *):
//
//   LEFT SIDE              RIGHT SIDE
//   ──────────             ──────────
//   D0* = GPIO1  (BCLK)    5V
//   D1* = GPIO2  (LRC)     GND*
//   D2* = GPIO3  (DOUT)    3V3*
//   D3* = GPIO4  (ENC_CLK) D10* = GPIO9  (LED_R)
//   D4* = GPIO5  (ENC_DT)  D9*  = GPIO8  (LED_G)
//   D5* = GPIO6  (ENC_SW)  D8*  = GPIO7  (LED_B)
//   D6  = GPIO43 (free)    D7   = GPIO44 (free)

// RGB LED (accent feedback during playback)
// Connect each pin through a 220-470Ω resistor to the LED
// Common cathode: connect LED common to GND, set RGB_ACTIVE_LOW = false
// Common anode: connect LED common to 3V3, set RGB_ACTIVE_LOW = true
#define RGB_ACTIVE_LOW      false
#define PIN_LED_RED         9     // D10: Red channel
#define PIN_LED_GREEN       8     // D9:  Green channel
#define PIN_LED_BLUE        7     // D8:  Blue channel

// I2S Audio (digital audio to MAX98357A DAC/Amplifier)
// These carry the digital audio signal - amp converts to analog for speaker
#define PIN_I2S_BCLK        1     // D0: Bit clock - timing for each audio bit
#define PIN_I2S_LRC         2     // D1: Left/Right clock - stereo channel select
#define PIN_I2S_DOUT        3     // D2: Data out - actual audio samples

// Rotary Encoder (KY-040 style — rotation + push button in one component)
// CLK and DT together detect rotation direction
// SW is the push button activated by pressing down on the encoder knob
#define PIN_ENC_CLK         4     // D3: Rotation clock (pulse per click)
#define PIN_ENC_DT          5     // D4: Rotation direction (phase vs CLK)
#define PIN_ENC_SW          6     // D5: Push button (active LOW when pressed)

// SD Card (Sense expansion board - no external wiring)
#define PIN_SD_CS           21    // Internal to expansion board

// ═══════════════════════════════════════════════════════════════════════════════
// CONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════════

#define SERIAL_BAUD         115200

// Audio
#define NUM_TRACKS          9
#define VOLUME_DEFAULT      15    // Startup volume (0-21)
#define VOLUME_MIN          0     // Mute
#define VOLUME_MAX          21    // Maximum (library limit)
#define VOLUME_STEP         1     // Change per encoder click

// Timing (ms)
#define DEBOUNCE_BUTTON_MS  50
#define DEBOUNCE_ENCODER_MS 5
#define LONG_PRESS_MS       500
#define TRACK_COLOR_MS      1500
#define VOLUME_FLASH_MS     200
#define SLEEP_POLL_MS       50

// LED Animation
#define LED_PWM_FREQ        5000
#define LED_PWM_BITS        8
#define BREATH_MIN          10
#define BREATH_MAX          255
#define BREATH_SPEED        2
#define RAINBOW_INTERVAL_MS 50

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
  {255,   0,   0},  // 1: Red
  {255, 127,   0},  // 2: Orange
  {255, 255,   0},  // 3: Yellow
  {  0, 255,   0},  // 4: Green
  {  0, 255, 255},  // 5: Cyan
  {  0,   0, 255},  // 6: Blue
  {127,   0, 255},  // 7: Purple
  {255,   0, 127},  // 8: Pink
  {255, 255, 255}   // 9: White
};

// ═══════════════════════════════════════════════════════════════════════════════
// TYPES & STATE
// ═══════════════════════════════════════════════════════════════════════════════

enum PlayerState { STATE_IDLE, STATE_PLAYING, STATE_PAUSED, STATE_ERROR };
static const char* const STATE_NAMES[] = { "IDLE", "PLAYING", "PAUSED", "ERROR" };

static Audio audio;

static PlayerState g_state    = STATE_IDLE;
static int         g_track    = 0;
static int         g_volume   = VOLUME_DEFAULT;
static bool        g_sdReady  = false;
static bool        g_sleeping = false;

static int           g_encLastCLK  = HIGH;
static unsigned long g_encLastTime = 0;

static unsigned long g_swPressStart = 0;
static bool          g_swPressed    = false;
static bool          g_swHandled    = false;

static float         g_breathPhase       = 0.0f;
static uint16_t      g_rainbowHue        = 0;
static unsigned long g_lastAnimTime      = 0;
static unsigned long g_trackColorStart   = 0;
static bool          g_showingTrackColor = false;
static unsigned long g_volFlashStart     = 0;
static bool          g_showingVolFlash   = false;

static unsigned long g_startTime = 0;

// ═══════════════════════════════════════════════════════════════════════════════
// SERIAL OUTPUT
// ═══════════════════════════════════════════════════════════════════════════════

static void printTimestamp() {
  unsigned long elapsed = millis() - g_startTime;
  unsigned long secs = elapsed / 1000;
  unsigned long mins = secs / 60;
  Serial.printf("[%02lu:%02lu.%03lu] ", mins, secs % 60, elapsed % 1000);
}

static void printHeader() {
  Serial.println();
  Serial.println(F("╔═══════════════════════════════════════════════════════════╗"));
  Serial.println(F("║     ♪ ♫  XIAO ESP32S3 Sense MP3 Player  ♫ ♪              ║"));
  Serial.println(F("║          ~ A magical little music box ~                   ║"));
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
  int filled = (vol * 21) / VOLUME_MAX;
  for (int i = 0; i < 21; i++) Serial.print(i < filled ? '=' : '-');
  Serial.printf("] %d/%d\n", vol, VOLUME_MAX);
}

static void printTrackInfo(int idx) {
  Serial.println();
  Serial.println(F("────────────────────────────────────────────────────────────"));
  printTimestamp();
  Serial.printf("NOW  Playing Track %d of %d\n", idx + 1, NUM_TRACKS);
  printTimestamp();
  Serial.printf("     File:  %s\n", TRACK_FILES[idx]);
  printTimestamp();
  Serial.printf("     Color: %s\n", TRACK_COLOR_NAMES[idx]);
  Serial.println(F("────────────────────────────────────────────────────────────"));
  Serial.println();
}

static void printStateChange(PlayerState from, PlayerState to) {
  printTimestamp();
  Serial.printf(">>>  State: %s --> %s\n", STATE_NAMES[from], STATE_NAMES[to]);
}

static void printWelcome() {
  Serial.println();
  Serial.println(F("┌───────────────────────────────────────────────────────────┐"));
  Serial.println(F("│  [OK] System ready!                                       │"));
  Serial.println(F("│                                                           │"));
  Serial.println(F("│  CONTROLS:                                                │"));
  Serial.println(F("│    Rotate ────────> Volume Up/Down                        │"));
  Serial.println(F("│    Short press ───> Play / Pause                          │"));
  Serial.println(F("│    Long press ────> Next Track                            │"));
  Serial.println(F("│                                                           │"));
  Serial.println(F("│  WIRING CHECK:                                            │"));
  Serial.println(F("│    D0/GPIO1 -> BCLK    D8/GPIO7 -> Blue LED               │"));
  Serial.println(F("│    D1/GPIO2 -> LRC     D9/GPIO8 -> Green LED              │"));
  Serial.println(F("│    D2/GPIO3 -> DIN     D10/GPIO9-> Red LED                │"));
  Serial.println(F("│    D3/GPIO4 -> CLK     3V3 -> Encoder VCC, Amp VIN        │"));
  Serial.println(F("│    D4/GPIO5 -> DT      GND -> All grounds                 │"));
  Serial.println(F("│    D5/GPIO6 -> SW                                         │"));
  Serial.println(F("│                                                           │"));
  Serial.println(F("│  Press encoder to start...                                │"));
  Serial.println(F("└───────────────────────────────────────────────────────────┘"));
  Serial.println();
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

// ═══════════════════════════════════════════════════════════════════════════════
// LED CONTROL
// ═══════════════════════════════════════════════════════════════════════════════

static void setLED(uint8_t r, uint8_t g, uint8_t b) {
  if (RGB_ACTIVE_LOW) {
    ledcWrite(PIN_LED_RED, 255 - r);
    ledcWrite(PIN_LED_GREEN, 255 - g);
    ledcWrite(PIN_LED_BLUE, 255 - b);
  } else {
    ledcWrite(PIN_LED_RED, r);
    ledcWrite(PIN_LED_GREEN, g);
    ledcWrite(PIN_LED_BLUE, b);
  }
}

static void hsvToRgb(uint16_t h, uint8_t s, uint8_t v, uint8_t* r, uint8_t* g, uint8_t* b) {
  if (s == 0) { *r = *g = *b = v; return; }
  uint8_t region = h / 60;
  uint8_t rem = (h - (region * 60)) * 255 / 60;
  uint8_t p = (v * (255 - s)) >> 8;
  uint8_t q = (v * (255 - ((s * rem) >> 8))) >> 8;
  uint8_t t = (v * (255 - ((s * (255 - rem)) >> 8))) >> 8;
  switch (region) {
    case 0:  *r = v; *g = t; *b = p; break;
    case 1:  *r = q; *g = v; *b = p; break;
    case 2:  *r = p; *g = v; *b = t; break;
    case 3:  *r = p; *g = q; *b = v; break;
    case 4:  *r = t; *g = p; *b = v; break;
    default: *r = v; *g = p; *b = q; break;
  }
}

static void showTrackColor(int idx) {
  if (idx < 0 || idx >= NUM_TRACKS) return;
  setLED(TRACK_COLORS[idx][0], TRACK_COLORS[idx][1], TRACK_COLORS[idx][2]);
  g_showingTrackColor = true;
  g_trackColorStart = millis();
}

static void showVolumeFlash() {
  setLED(100, 100, 100);
  g_showingVolFlash = true;
  g_volFlashStart = millis();
}

static void breathingRainbow() {
  g_breathPhase += 0.02f * BREATH_SPEED;
  if (g_breathPhase > TWO_PI) g_breathPhase -= TWO_PI;
  float breathVal = (sinf(g_breathPhase) + 1.0f) / 2.0f;
  uint8_t brightness = BREATH_MIN + (uint8_t)(breathVal * (BREATH_MAX - BREATH_MIN));
  g_rainbowHue = (g_rainbowHue + 1) % 360;
  uint8_t r, g, b;
  hsvToRgb(g_rainbowHue, 255, brightness, &r, &g, &b);
  setLED(r, g, b);
}

static void updateLED() {
  unsigned long now = millis();
  
  if (g_showingVolFlash) {
    if (now - g_volFlashStart >= VOLUME_FLASH_MS) g_showingVolFlash = false;
    else return;
  }
  
  if (g_showingTrackColor) {
    if (now - g_trackColorStart >= TRACK_COLOR_MS) {
      g_showingTrackColor = false;
      g_breathPhase = 0.0f;
    } else return;
  }
  
  switch (g_state) {
    case STATE_PLAYING:
      if (now - g_lastAnimTime >= RAINBOW_INTERVAL_MS) {
        g_lastAnimTime = now;
        breathingRainbow();
      }
      break;
    case STATE_PAUSED:
    case STATE_IDLE:
      setLED(0, 0, 30);
      break;
    case STATE_ERROR: {
      g_breathPhase += 0.1f;
      if (g_breathPhase > TWO_PI) g_breathPhase -= TWO_PI;
      uint8_t br = 50 + (uint8_t)((sinf(g_breathPhase) + 1.0f) * 100.0f);
      setLED(br, 0, 0);
      break;
    }
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// SLEEP / WAKE
// ═══════════════════════════════════════════════════════════════════════════════

static void enterSleep() {
  g_sleeping = true;
  setLED(0, 0, 0);
  Serial.println();
  printStatus("zzz", "Entering sleep... (press/rotate to wake)");
  Serial.println();
}

static void wakeUp() {
  Serial.println();
  printStatus("***", "Waking up!");
  g_sleeping = false;
  delay(100);
  g_encLastCLK = digitalRead(PIN_ENC_CLK);
  g_swPressed = false;
  g_swHandled = false;
  
  if (g_state == STATE_PAUSED) {
    audio.pauseResume();
    PlayerState old = g_state;
    g_state = STATE_PLAYING;
    printStateChange(old, g_state);
    printStatus("PLAY", "Resumed");
  } else if (g_state == STATE_IDLE) {
    playTrack(g_track);
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// SD CARD
// ═══════════════════════════════════════════════════════════════════════════════

static bool fileExists(const char* path) {
  File f = SD.open(path);
  if (f) { bool ok = !f.isDirectory(); f.close(); return ok; }
  return false;
}

static void setupSD() {
  printStatus("SD", "Initializing...");
  if (!SD.begin(PIN_SD_CS)) {
    printError("SD mount failed! Check: inserted? FAT32?");
    g_sdReady = false;
    return;
  }
  uint8_t type = SD.cardType();
  if (type == CARD_NONE) {
    printError("No SD card detected!");
    g_sdReady = false;
    return;
  }
  const char* typeStr = (type == CARD_SDHC) ? "SDHC" : (type == CARD_SD) ? "SD" : "MMC";
  printTimestamp();
  Serial.printf("SD   Card: %s, %llu MB\n", typeStr, SD.cardSize() / (1024 * 1024));
  g_sdReady = true;
  printSuccess("SD ready");
}

static void scanTracks() {
  printStatus("SCAN", "Looking for tracks...");
  int found = 0;
  for (int i = 0; i < NUM_TRACKS; i++) {
    bool exists = fileExists(TRACK_FILES[i]);
    printTimestamp();
    Serial.printf("     [%d] %s %s\n", i + 1, TRACK_FILES[i], exists ? "[OK]" : "[MISSING]");
    if (exists) found++;
  }
  printTimestamp();
  Serial.printf("STAT Found %d of %d tracks\n", found, NUM_TRACKS);
  Serial.println();
}

// ═══════════════════════════════════════════════════════════════════════════════
// AUDIO
// ═══════════════════════════════════════════════════════════════════════════════

static void setupAudio() {
  printStatus("AUD", "Initializing...");
  audio.setPinout(PIN_I2S_BCLK, PIN_I2S_LRC, PIN_I2S_DOUT);
  audio.setVolume(g_volume);
  printSuccess("Audio ready");
  printVolumeBar(g_volume);
}

static void playTrack(int idx) {
  if (!g_sdReady) { printError("SD not ready!"); g_state = STATE_ERROR; return; }
  if (idx < 0 || idx >= NUM_TRACKS) idx = 0;
  
  g_track = idx;
  showTrackColor(g_track);
  
  if (!fileExists(TRACK_FILES[g_track])) {
    printStatusF("WARN", "Track %d missing, searching...", g_track + 1);
    int tries = 0;
    while (!fileExists(TRACK_FILES[g_track]) && tries < NUM_TRACKS) {
      g_track = (g_track + 1) % NUM_TRACKS;
      tries++;
    }
    if (tries >= NUM_TRACKS) {
      printError("No tracks found!");
      g_state = STATE_ERROR;
      setLED(255, 0, 0);
      return;
    }
    showTrackColor(g_track);
  }
  
  if (audio.connecttoFS(SD, TRACK_FILES[g_track])) {
    PlayerState old = g_state;
    g_state = STATE_PLAYING;
    printStateChange(old, g_state);
    printTrackInfo(g_track);
  } else {
    printError("Playback failed!");
    g_state = STATE_ERROR;
    setLED(255, 0, 0);
  }
}

static void nextTrack() {
  printStatus("SKIP", "Next track...");
  audio.stopSong();
  playTrack((g_track + 1) % NUM_TRACKS);
}

static void togglePlayPause() {
  switch (g_state) {
    case STATE_IDLE:
      printStatus("PLAY", "Starting...");
      playTrack(g_track);
      break;
    case STATE_PLAYING: {
      audio.pauseResume();
      PlayerState old = g_state;
      g_state = STATE_PAUSED;
      printStateChange(old, g_state);
      printStatus("PAUS", "Paused");
      enterSleep();
      break;
    }
    case STATE_PAUSED: {
      audio.pauseResume();
      PlayerState old = g_state;
      g_state = STATE_PLAYING;
      printStateChange(old, g_state);
      printStatus("PLAY", "Resumed");
      break;
    }
    case STATE_ERROR:
      printStatus("RTRY", "Retrying...");
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
// INPUT
// ═══════════════════════════════════════════════════════════════════════════════

static void handleEncoder() {
  int clk = digitalRead(PIN_ENC_CLK);
  if (millis() - g_encLastTime < DEBOUNCE_ENCODER_MS) return;
  if (clk != g_encLastCLK && clk == LOW) {
    g_encLastTime = millis();
    adjustVolume((digitalRead(PIN_ENC_DT) != clk) ? VOLUME_STEP : -VOLUME_STEP);
  }
  g_encLastCLK = clk;
}

static void handleEncoderSwitch() {
  bool pressed = !digitalRead(PIN_ENC_SW);  // Active LOW
  unsigned long now = millis();
  
  // Switch just pressed
  if (pressed && !g_swPressed) {
    g_swPressStart = now;
    g_swPressed = true;
    g_swHandled = false;
  }
  
  // Switch being held - check for long press
  if (pressed && g_swPressed && !g_swHandled) {
    if (now - g_swPressStart >= LONG_PRESS_MS) {
      printStatus("ENC", "Long press → Next track");
      nextTrack();
      g_swHandled = true;
    }
  }
  
  // Switch released
  if (!pressed && g_swPressed) {
    if (!g_swHandled && (now - g_swPressStart) >= DEBOUNCE_BUTTON_MS) {
      printStatus("ENC", "Short press → Play/Pause");
      togglePlayPause();
    }
    g_swPressed = false;
    g_swHandled = false;
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// SETUP & LOOP
// ═══════════════════════════════════════════════════════════════════════════════

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1000);
  g_startTime = millis();
  
  printHeader();
  printStatus("INIT", "Starting...");
  
  // GPIO
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_ENC_CLK, INPUT_PULLUP);
  pinMode(PIN_ENC_DT, INPUT_PULLUP);
  pinMode(PIN_ENC_SW, INPUT_PULLUP);
  g_encLastCLK = digitalRead(PIN_ENC_CLK);
  printSuccess("GPIO ready");
  
  // LED PWM
  ledcAttach(PIN_LED_RED, LED_PWM_FREQ, LED_PWM_BITS);
  ledcAttach(PIN_LED_GREEN, LED_PWM_FREQ, LED_PWM_BITS);
  ledcAttach(PIN_LED_BLUE, LED_PWM_FREQ, LED_PWM_BITS);
  setLED(128, 0, 128);
  printSuccess("LED ready");
  
  // SD & Audio
  setupSD();
  if (g_sdReady) {
    setupAudio();
    scanTracks();
    g_track = 0;
    g_state = STATE_IDLE;
    Serial.println(F("════════════════════════════════════════════════════════════"));
    printWelcome();
    Serial.println(F("════════════════════════════════════════════════════════════"));
    enterSleep();
  } else {
    g_state = STATE_ERROR;
    setLED(255, 0, 0);
  }
}

void loop() {
  if (g_sleeping) {
    if (!digitalRead(PIN_ENC_SW) || digitalRead(PIN_ENC_CLK) != g_encLastCLK) wakeUp();
    delay(SLEEP_POLL_MS);
    return;
  }
  audio.loop();
  handleEncoder();
  handleEncoderSwitch();
  updateLED();
}

// ═══════════════════════════════════════════════════════════════════════════════
// AUDIO CALLBACKS
// ═══════════════════════════════════════════════════════════════════════════════

void audio_eof_mp3(const char* info) {
  printStatus("END", "Track done");
  nextTrack();
}

void audio_id3data(const char* info)  { printStatusF("ID3", "%s", info); }
void audio_info(const char* info)     { /* Filtered - too verbose */ }
void audio_error(const char* info)    { printStatusF("ERR", "%s", info); g_state = STATE_ERROR; setLED(255,0,0); }
void audio_bitrate(const char* info)  { printStatusF("RATE", "%s", info); }

void audio_commercial(const char* i)      { (void)i; }
void audio_icyurl(const char* i)          { (void)i; }
void audio_lasthost(const char* i)        { (void)i; }
void audio_showstation(const char* i)     { (void)i; }
void audio_showstreamtitle(const char* i) { (void)i; }
