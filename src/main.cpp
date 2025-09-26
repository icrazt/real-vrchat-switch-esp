#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <OneButton.h>

#include <cstddef>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "strandtest_nodelay.h"

#define KEY_USER_2     2
#define KEY_USER_MAIN  4
#define KEY_USER_BOOT  9

#define PIN_LED        8
#define PIN_RGB        3
#define PIN_RGB_EN     10
#define RGB_NUM        22

OneButton main_button(KEY_USER_MAIN);
Adafruit_NeoPixel strip(RGB_NUM, PIN_RGB, NEO_GRB + NEO_KHZ800);
StrandtestController strandtest(strip);

namespace {

constexpr uint8_t kBrightnessHigh = 200;
constexpr uint8_t kBrightnessLow = 40;

constexpr TickType_t kButtonTaskDelay = pdMS_TO_TICKS(5);
constexpr TickType_t kRgbTaskDelay = pdMS_TO_TICKS(10);

constexpr unsigned long kBreathingIntervalMs = 30;
constexpr uint8_t kSolidColorR = 255;
constexpr uint8_t kSolidColorG = 140;
constexpr uint8_t kSolidColorB = 0;

enum class BrightnessMode : uint8_t {
  kBright = 0,
  kDim,
};

enum class GlowMode : uint8_t {
  kSolid = 0,
  kBreathing,
  kRainbow,
  kTheaterChase,
  kTheaterChaseRainbow,
};

enum class ButtonEventType : uint8_t {
  kSingleClick = 0,
  kLongPress,
};

struct ButtonEvent {
  ButtonEventType type;
};

struct BreathingState {
  uint8_t brightness;
  int8_t direction;
  unsigned long last_update_ms;
};

constexpr GlowMode kGlowModes[] = {
    GlowMode::kSolid,
    GlowMode::kBreathing,
    GlowMode::kRainbow,
    GlowMode::kTheaterChase,
    GlowMode::kTheaterChaseRainbow,
};
constexpr std::size_t kGlowModeCount = sizeof(kGlowModes) / sizeof(kGlowModes[0]);

QueueHandle_t button_event_queue = nullptr;

uint8_t BrightnessForMode(BrightnessMode mode) {
  return mode == BrightnessMode::kBright ? kBrightnessHigh : kBrightnessLow;
}

uint8_t BreathingMinimum(BrightnessMode mode) {
  return mode == BrightnessMode::kBright ? 20 : 5;
}

uint8_t BreathingMaximum(BrightnessMode mode) {
  return mode == BrightnessMode::kBright ? kBrightnessHigh : 80;
}

uint8_t BreathingStep(BrightnessMode mode) {
  return mode == BrightnessMode::kBright ? 3 : 1;
}

uint32_t MakeColor(uint8_t r, uint8_t g, uint8_t b) {
  return strip.Color(r, g, b);
}

void PublishButtonEvent(ButtonEventType type) {
  if (!button_event_queue) {
    return;
  }
  ButtonEvent event{type};
  xQueueSend(button_event_queue, &event, 0);
}

void SyncBrightness(BrightnessMode brightness_mode,
                    GlowMode glow_mode,
                    BreathingState &breathing_state,
                    bool &solid_needs_refresh) {
  const uint8_t brightness = BrightnessForMode(brightness_mode);
  switch (glow_mode) {
    case GlowMode::kSolid:
      strip.setBrightness(brightness);
      strandtest.setBrightness(brightness);
      solid_needs_refresh = true;
      break;
    case GlowMode::kBreathing:
      breathing_state.brightness = BreathingMaximum(brightness_mode);
      breathing_state.direction = -1;
      breathing_state.last_update_ms = millis();
      strip.setBrightness(breathing_state.brightness);
      strip.fill(MakeColor(kSolidColorR, kSolidColorG, kSolidColorB), 0, strip.numPixels());
      strip.show();
      break;
    case GlowMode::kRainbow:
    case GlowMode::kTheaterChase:
    case GlowMode::kTheaterChaseRainbow:
    default:
      strandtest.setBrightness(brightness);
      break;
  }
}

void ConfigureForMode(GlowMode glow_mode,
                      BrightnessMode brightness_mode,
                      BreathingState &breathing_state,
                      bool &solid_needs_refresh) {
  switch (glow_mode) {
    case GlowMode::kSolid:
      strandtest.setAutoCycle(false);
      SyncBrightness(brightness_mode, glow_mode, breathing_state, solid_needs_refresh);
      break;
    case GlowMode::kBreathing:
      strandtest.setAutoCycle(false);
      SyncBrightness(brightness_mode, glow_mode, breathing_state, solid_needs_refresh);
      break;
    case GlowMode::kRainbow:
      strandtest.setAutoCycle(false);
      strandtest.setPattern(StrandPattern::kRainbow);
      strandtest.setRainbowWait(8);
      SyncBrightness(brightness_mode, glow_mode, breathing_state, solid_needs_refresh);
      break;
    case GlowMode::kTheaterChase:
      strandtest.setAutoCycle(false);
      strandtest.setPattern(StrandPattern::kTheaterChase,
                            MakeColor(kSolidColorR, kSolidColorG, kSolidColorB));
      strandtest.setTheaterChaseWait(50);
      SyncBrightness(brightness_mode, glow_mode, breathing_state, solid_needs_refresh);
      break;
    case GlowMode::kTheaterChaseRainbow:
      strandtest.setAutoCycle(false);
      strandtest.setPattern(StrandPattern::kTheaterChaseRainbow);
      strandtest.setTheaterChaseRainbowWait(40);
      SyncBrightness(brightness_mode, glow_mode, breathing_state, solid_needs_refresh);
      break;
    default:
      break;
  }
}

void UpdateBreathing(BrightnessMode brightness_mode, BreathingState &breathing_state) {
  const unsigned long now = millis();
  if ((now - breathing_state.last_update_ms) < kBreathingIntervalMs) {
    return;
  }
  breathing_state.last_update_ms = now;

  const uint8_t min_brightness = BreathingMinimum(brightness_mode);
  const uint8_t max_brightness = BreathingMaximum(brightness_mode);
  const uint8_t step = BreathingStep(brightness_mode);

  if (breathing_state.direction > 0) {
    if (breathing_state.brightness + step < max_brightness) {
      breathing_state.brightness = breathing_state.brightness + step;
    } else {
      breathing_state.brightness = max_brightness;
      breathing_state.direction = -1;
    }
  } else {
    if (breathing_state.brightness > min_brightness + step) {
      breathing_state.brightness = breathing_state.brightness - step;
    } else {
      breathing_state.brightness = min_brightness;
      breathing_state.direction = 1;
    }
  }

  strip.setBrightness(breathing_state.brightness);
  strip.show();
}

}  // namespace

void ButtonClick(void *context) {
  PublishButtonEvent(ButtonEventType::kSingleClick);
}

void LongPressStop(void *context) {
  PublishButtonEvent(ButtonEventType::kLongPress);
}

void TaskButton(void *param) {
  main_button.attachClick(ButtonClick, &main_button);
  main_button.attachLongPressStop(LongPressStop, &main_button);
  main_button.setLongPressIntervalMs(1000);

  for (;;) {
    main_button.tick();
    vTaskDelay(kButtonTaskDelay);
  }
}

void TaskRGB(void *param) {
  BrightnessMode requested_brightness = BrightnessMode::kBright;
  GlowMode requested_glow = GlowMode::kSolid;
  std::size_t glow_mode_index = 0;

  BrightnessMode applied_brightness = requested_brightness;
  GlowMode applied_glow = requested_glow;

  BreathingState breathing_state{};
  breathing_state.brightness = BreathingMaximum(applied_brightness);
  breathing_state.direction = -1;
  breathing_state.last_update_ms = millis();

  bool solid_needs_refresh = true;

  digitalWrite(PIN_RGB_EN, HIGH);
  strip.begin();
  strip.show();
  strandtest.begin();
  strandtest.setAutoCycle(false);

  SyncBrightness(applied_brightness, applied_glow, breathing_state, solid_needs_refresh);
  ConfigureForMode(applied_glow, applied_brightness, breathing_state, solid_needs_refresh);

  for (;;) {
    ButtonEvent event;
    while (xQueueReceive(button_event_queue, &event, 0) == pdPASS) {
      switch (event.type) {
        case ButtonEventType::kSingleClick:
          requested_brightness = (requested_brightness == BrightnessMode::kBright)
                                     ? BrightnessMode::kDim
                                     : BrightnessMode::kBright;
          break;
        case ButtonEventType::kLongPress:
          glow_mode_index = (glow_mode_index + 1) % kGlowModeCount;
          requested_glow = kGlowModes[glow_mode_index];
          break;
        default:
          break;
      }
    }

    if (requested_brightness != applied_brightness) {
      applied_brightness = requested_brightness;
      SyncBrightness(applied_brightness, applied_glow, breathing_state, solid_needs_refresh);
    }

    if (requested_glow != applied_glow) {
      applied_glow = requested_glow;
      ConfigureForMode(applied_glow, applied_brightness, breathing_state, solid_needs_refresh);
    }

    switch (applied_glow) {
      case GlowMode::kSolid: {
        if (solid_needs_refresh) {
          const uint32_t color = MakeColor(kSolidColorR, kSolidColorG, kSolidColorB);
          strip.fill(color, 0, strip.numPixels());
          strip.show();
          solid_needs_refresh = false;
        }
        break;
      }
      case GlowMode::kBreathing:
        UpdateBreathing(applied_brightness, breathing_state);
        break;
      case GlowMode::kRainbow:
      case GlowMode::kTheaterChase:
      case GlowMode::kTheaterChaseRainbow:
        strandtest.update();
        break;
      default:
        break;
    }

    vTaskDelay(kRgbTaskDelay);
  }
}

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_RGB_EN, OUTPUT);
  pinMode(KEY_USER_MAIN, INPUT_PULLUP);

  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_RGB_EN, HIGH);

  Serial.begin(115200);

  button_event_queue = xQueueCreate(8, sizeof(ButtonEvent));
  if (button_event_queue == nullptr) {
    Serial.println("Failed to create button event queue.");
    while (true) {
      delay(1000);
    }
  }

  xTaskCreate(TaskButton, "TaskButton", 2048, nullptr, 3, nullptr);
  xTaskCreate(TaskRGB, "TaskRGB", 4096, nullptr, 2, nullptr);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
