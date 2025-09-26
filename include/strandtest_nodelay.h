#pragma once

#include <Adafruit_NeoPixel.h>

class StrandtestController {
 public:
  explicit StrandtestController(Adafruit_NeoPixel &strip);

  void begin();
  void update();

 private:
  void colorWipe(uint32_t color, int wait);
  void theaterChase(uint32_t color, int wait);
  uint32_t wheel(uint8_t wheel_pos);
  void rainbow(uint8_t wait);
  void theaterChaseRainbow(uint8_t wait);

  Adafruit_NeoPixel &strip_;
  unsigned long pixel_previous_;
  unsigned long pattern_previous_;
  int pattern_current_;
  int pattern_interval_;
  bool pattern_complete_;

  int pixel_interval_;
  int pixel_queue_;
  int pixel_cycle_;
  uint16_t pixel_number_;

  uint16_t color_wipe_position_;
  uint16_t theater_chase_offset_;
  uint32_t theater_chase_loops_;
};
