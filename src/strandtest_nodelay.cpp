#include <Arduino.h>

#include "strandtest_nodelay.h"

namespace {
constexpr int kDefaultPatternIntervalMs = 5000;
constexpr int kDefaultColorPatternWaitMs = 50;
constexpr uint8_t kDefaultRainbowWaitMs = 10;
constexpr uint8_t kDefaultTheaterChaseRainbowWaitMs = 50;
constexpr uint8_t kDefaultBrightness = 50;
constexpr int kTheaterChaseLoopTarget = 10;
constexpr int kTheaterChaseStride = 3;

struct CycleEntry {
  StrandPattern pattern;
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

constexpr CycleEntry kDefaultCycle[] = {
    {StrandPattern::kColorWipe, 255, 0, 0},
    {StrandPattern::kColorWipe, 0, 255, 0},
    {StrandPattern::kColorWipe, 0, 0, 255},
    {StrandPattern::kTheaterChase, 127, 127, 127},
    {StrandPattern::kTheaterChase, 127, 0, 0},
    {StrandPattern::kTheaterChase, 0, 0, 127},
    {StrandPattern::kRainbow, 0, 0, 0},
    {StrandPattern::kTheaterChaseRainbow, 0, 0, 0},
};

constexpr std::size_t kDefaultCycleLength = sizeof(kDefaultCycle) / sizeof(kDefaultCycle[0]);
}  // namespace

StrandtestController::StrandtestController(Adafruit_NeoPixel &strip)
    : strip_(strip),
      pixel_previous_(0),
      pattern_previous_(0),
      pattern_current_(StrandPattern::kColorWipe),
      pattern_interval_(kDefaultPatternIntervalMs),
      pattern_complete_(false),
      auto_cycle_(true),
      pattern_index_(0),
      force_refresh_(true),
      primary_color_(strip.Color(255, 0, 0)),
      color_wipe_wait_(kDefaultColorPatternWaitMs),
      theater_chase_wait_(kDefaultColorPatternWaitMs),
      rainbow_wait_(kDefaultRainbowWaitMs),
      theater_chase_rainbow_wait_(kDefaultTheaterChaseRainbowWaitMs),
      pixel_interval_(kDefaultColorPatternWaitMs),
      pixel_queue_(0),
      pixel_cycle_(0),
      pixel_number_(strip.numPixels()),
      color_wipe_position_(0),
      theater_chase_offset_(0),
      theater_chase_loops_(0) {}

void StrandtestController::begin() {
  strip_.begin();
  strip_.show();
  strip_.setBrightness(kDefaultBrightness);

  const unsigned long now = millis();
  if (auto_cycle_) {
    applyDefaultCycleEntry(pattern_index_, now);
  } else {
    applyPattern(pattern_current_, now);
  }
}

void StrandtestController::update() {
  const unsigned long current_millis = millis();
  handleAutoCycle(current_millis);

  if (force_refresh_ ||
      (current_millis - pixel_previous_) >= static_cast<unsigned long>(pixel_interval_)) {
    pixel_previous_ = current_millis;
    force_refresh_ = false;

    switch (pattern_current_) {
      case StrandPattern::kTheaterChaseRainbow:
        theaterChaseRainbow(theater_chase_rainbow_wait_);
        break;
      case StrandPattern::kRainbow:
        rainbow(rainbow_wait_);
        break;
      case StrandPattern::kTheaterChase:
        theaterChase(primary_color_, theater_chase_wait_);
        break;
      case StrandPattern::kColorWipe:
      default:
        colorWipe(primary_color_, color_wipe_wait_);
        break;
    }
  }
}

void StrandtestController::setAutoCycle(bool enabled) {
  if (auto_cycle_ == enabled) {
    return;
  }
  auto_cycle_ = enabled;
  const unsigned long now = millis();
  pattern_previous_ = now;
  if (auto_cycle_) {
    pattern_index_ = 0;
    applyDefaultCycleEntry(pattern_index_, now);
  } else {
    applyPattern(pattern_current_, now);
  }
}

void StrandtestController::setPattern(StrandPattern pattern) {
  const unsigned long now = millis();
  applyPattern(pattern, now);
}

void StrandtestController::setPattern(StrandPattern pattern, uint32_t color) {
  primary_color_ = color;
  setPattern(pattern);
}

void StrandtestController::setPrimaryColor(uint32_t color) {
  primary_color_ = color;
  resetPatternState();
}

void StrandtestController::setPatternInterval(int interval_ms) {
  if (interval_ms < 0) {
    interval_ms = 0;
  }
  pattern_interval_ = interval_ms;
  pattern_previous_ = millis();
}

void StrandtestController::setColorWipeWait(int wait_ms) {
  if (wait_ms < 0) {
    wait_ms = 0;
  }
  color_wipe_wait_ = wait_ms;
  if (pattern_current_ == StrandPattern::kColorWipe) {
    resetPatternState();
  }
}

void StrandtestController::setTheaterChaseWait(int wait_ms) {
  if (wait_ms < 0) {
    wait_ms = 0;
  }
  theater_chase_wait_ = wait_ms;
  if (pattern_current_ == StrandPattern::kTheaterChase) {
    resetPatternState();
  }
}

void StrandtestController::setRainbowWait(uint8_t wait_ms) {
  rainbow_wait_ = wait_ms;
  if (pattern_current_ == StrandPattern::kRainbow) {
    resetPatternState();
  }
}

void StrandtestController::setTheaterChaseRainbowWait(uint8_t wait_ms) {
  theater_chase_rainbow_wait_ = wait_ms;
  if (pattern_current_ == StrandPattern::kTheaterChaseRainbow) {
    resetPatternState();
  }
}

void StrandtestController::setBrightness(uint8_t brightness) {
  strip_.setBrightness(brightness);
  strip_.show();
}

void StrandtestController::colorWipe(uint32_t color, int wait) {
  pixel_interval_ = wait;
  strip_.setPixelColor(color_wipe_position_++, color);
  strip_.show();
  if (color_wipe_position_ >= pixel_number_) {
    color_wipe_position_ = 0;
    pattern_complete_ = true;
  }
}

void StrandtestController::theaterChase(uint32_t color, int wait) {
  pixel_interval_ = wait;

  strip_.clear();
  for (int c = theater_chase_offset_; c < pixel_number_; c += kTheaterChaseStride) {
    strip_.setPixelColor(c, color);
  }
  strip_.show();

  theater_chase_offset_++;
  if (theater_chase_offset_ >= kTheaterChaseStride) {
    theater_chase_offset_ = 0;
    theater_chase_loops_++;
  }

  if (theater_chase_loops_ >= kTheaterChaseLoopTarget) {
    theater_chase_offset_ = 0;
    theater_chase_loops_ = 0;
    pattern_complete_ = true;
  }
}

uint32_t StrandtestController::wheel(uint8_t wheel_pos) {
  wheel_pos = 255 - wheel_pos;
  if (wheel_pos < 85) {
    return strip_.Color(255 - wheel_pos * 3, 0, wheel_pos * 3);
  }
  if (wheel_pos < 170) {
    wheel_pos -= 85;
    return strip_.Color(0, wheel_pos * 3, 255 - wheel_pos * 3);
  }
  wheel_pos -= 170;
  return strip_.Color(wheel_pos * 3, 255 - wheel_pos * 3, 0);
}

void StrandtestController::rainbow(uint8_t wait) {
  if (pixel_interval_ != wait) {
    pixel_interval_ = wait;
  }
  for (uint16_t i = 0; i < pixel_number_; i++) {
    strip_.setPixelColor(i, wheel((i + pixel_cycle_) & 255));
  }
  strip_.show();
  pixel_cycle_++;
  if (pixel_cycle_ >= 256) {
    pixel_cycle_ = 0;
  }
}

void StrandtestController::theaterChaseRainbow(uint8_t wait) {
  if (pixel_interval_ != wait) {
    pixel_interval_ = wait;
  }
  for (int i = 0; i < pixel_number_; i += kTheaterChaseStride) {
    const int index = i + pixel_queue_;
    if (index < pixel_number_) {
      strip_.setPixelColor(index, wheel((i + pixel_cycle_) % 255));
    }
  }
  strip_.show();
  for (int i = 0; i < pixel_number_; i += kTheaterChaseStride) {
    const int index = i + pixel_queue_;
    if (index < pixel_number_) {
      strip_.setPixelColor(index, strip_.Color(0, 0, 0));
    }
  }
  pixel_queue_++;
  pixel_cycle_++;
  if (pixel_queue_ >= kTheaterChaseStride) {
    pixel_queue_ = 0;
  }
  if (pixel_cycle_ >= 256) {
    pixel_cycle_ = 0;
  }
}

void StrandtestController::resetPatternState() {
  pattern_complete_ = false;
  force_refresh_ = true;
  pixel_queue_ = 0;
  pixel_cycle_ = 0;
  color_wipe_position_ = 0;
  theater_chase_offset_ = 0;
  theater_chase_loops_ = 0;
}

void StrandtestController::handleAutoCycle(unsigned long current_millis) {
  if (!auto_cycle_) {
    return;
  }
  if (pattern_complete_ ||
      (current_millis - pattern_previous_) >= static_cast<unsigned long>(pattern_interval_)) {
    pattern_index_ = (pattern_index_ + 1) % kDefaultCycleLength;
    applyDefaultCycleEntry(pattern_index_, current_millis);
  }
}

void StrandtestController::applyPattern(StrandPattern pattern, unsigned long current_millis) {
  pattern_current_ = pattern;
  pattern_previous_ = current_millis;
  resetPatternState();
}

void StrandtestController::applyDefaultCycleEntry(std::size_t index, unsigned long current_millis) {
  if (index >= kDefaultCycleLength) {
    index = 0;
  }
  const auto &entry = kDefaultCycle[index];
  if (entry.pattern == StrandPattern::kColorWipe || entry.pattern == StrandPattern::kTheaterChase) {
    primary_color_ = strip_.Color(entry.r, entry.g, entry.b);
  }
  applyPattern(entry.pattern, current_millis);
}
