#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "strandtest_nodelay.h"

// 按键定义
#define KEY_USER_2     2
#define KEY_USER_MAIN  4     // 主按键, 当前版本外壳唯一的操作按键
#define KEY_USER_BOOT  9     // BOOT模式切换按键，正常启动后也可作为用户按键
// LED定义
#define PIN_LED        8     // 白色指示灯引脚
#define PIN_RGB        3     // WS2812 灯珠数据引脚
#define PIN_RGB_EN     10    // WS2812 灯珠供电使能
#define RGB_NUM        22    // WS2812 灯珠数量

Adafruit_NeoPixel strip(RGB_NUM, PIN_RGB, NEO_GRB + NEO_KHZ800);
StrandtestController strandtest(strip);

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_RGB_EN, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_RGB_EN, HIGH);

  strandtest.begin();
}

void loop() {
  strandtest.update();
}
