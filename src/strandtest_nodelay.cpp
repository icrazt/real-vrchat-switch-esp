#include <Arduino.h>

#include "strandtest_nodelay.h"

namespace {
constexpr int kDefaultPatternIntervalMs = 5000;
constexpr int kDefaultPixelIntervalMs = 50;
constexpr int kTheaterChaseLoopTarget = 10;
constexpr int kTheaterChaseStride = 3;
}  // namespace

StrandtestController::StrandtestController(Adafruit_NeoPixel &strip)
    : strip_(strip),
      pixel_previous_(0), 
      pattern_previous_(0),
      pattern_current_(0),
      pattern_interval_(kDefaultPatternIntervalMs),
      pattern_complete_(false),
      pixel_interval_(kDefaultPixelIntervalMs),
      pixel_queue_(0),
      pixel_cycle_(0),
      pixel_number_(strip.numPixels()),
      color_wipe_position_(0),
      theater_chase_offset_(0),
      theater_chase_loops_(0) {}

void StrandtestController::begin() {
  strip_.begin();
  strip_.show();
  strip_.setBrightness(50);
}

void StrandtestController::update() {
  const unsigned long current_millis = millis();
  if (pattern_complete_ ||
      (current_millis - pattern_previous_) >= static_cast<unsigned long>(pattern_interval_)) {
    pattern_complete_ = false;
    pattern_previous_ = current_millis;
    pattern_current_++;
    if (pattern_current_ >= 7) {
      pattern_current_ = 0;
    }
  }

  if ((current_millis - pixel_previous_) >= static_cast<unsigned long>(pixel_interval_)) {
    pixel_previous_ = current_millis;
    switch (pattern_current_) {
      case 7:
        theaterChaseRainbow(50);
        break;
      case 6:
        rainbow(10);
        break;
      case 5:
        theaterChase(strip_.Color(0, 0, 127), 50);
        break;
      case 4:
        theaterChase(strip_.Color(127, 0, 0), 50);
        break;
      case 3:
        theaterChase(strip_.Color(127, 127, 127), 50);
        break;
      case 2:
        colorWipe(strip_.Color(0, 0, 255), 50);
        break;
      case 1:
        colorWipe(strip_.Color(0, 255, 0), 50);
        break;
      default:
        colorWipe(strip_.Color(255, 0, 0), 50);
        break;
    }
  }
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
