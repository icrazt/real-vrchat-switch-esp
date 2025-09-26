#pragma once

#include <Adafruit_NeoPixel.h>
#include <cstddef>
#include <cstdint>

enum class StrandPattern {
  kColorWipe = 0,
  kTheaterChase,
  kRainbow,
  kTheaterChaseRainbow,
};

class StrandtestController {
 public:
  explicit StrandtestController(Adafruit_NeoPixel &strip);

  void begin();
  void update();

  void setAutoCycle(bool enabled);
  void setPattern(StrandPattern pattern);
  void setPattern(StrandPattern pattern, uint32_t color);
  void setPrimaryColor(uint32_t color);
  void setPatternInterval(int interval_ms);
  void setColorWipeWait(int wait_ms);
  void setTheaterChaseWait(int wait_ms);
  void setRainbowWait(uint8_t wait_ms);
  void setTheaterChaseRainbowWait(uint8_t wait_ms);
  void setBrightness(uint8_t brightness);

 private:
  void colorWipe(uint32_t color, int wait);
  void theaterChase(uint32_t color, int wait);
  uint32_t wheel(uint8_t wheel_pos);
  void rainbow(uint8_t wait);
  void theaterChaseRainbow(uint8_t wait);

  void resetPatternState();
  void handleAutoCycle(unsigned long current_millis);
  void applyPattern(StrandPattern pattern, unsigned long current_millis);
  void applyDefaultCycleEntry(std::size_t index, unsigned long current_millis);

  Adafruit_NeoPixel &strip_;
  unsigned long pixel_previous_;
  unsigned long pattern_previous_;
  StrandPattern pattern_current_;
  int pattern_interval_;
  bool pattern_complete_;
  bool auto_cycle_;
  std::size_t pattern_index_;
  bool force_refresh_;

  uint32_t primary_color_;
  int color_wipe_wait_;
  int theater_chase_wait_;
  uint8_t rainbow_wait_;
  uint8_t theater_chase_rainbow_wait_;

  int pixel_interval_;
  int pixel_queue_;
  int pixel_cycle_;
  uint16_t pixel_number_;

  uint16_t color_wipe_position_;
  uint16_t theater_chase_offset_;
  uint32_t theater_chase_loops_;
};
