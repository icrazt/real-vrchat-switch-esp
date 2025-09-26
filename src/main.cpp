#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <OneButton.h>

#include "strandtest_nodelay.h"

// 按键定义
#define KEY_USER_2     2
#define KEY_USER_MAIN  4     // 主按键, 当前版本外壳唯一的操作按键
#define KEY_USER_BOOT  9     // BOOT模式切换按键，正常启动后也可作为用户按键

OneButton main_button(KEY_USER_MAIN);

// LED定义
#define PIN_LED        8     // 白色指示灯引脚
#define PIN_RGB        3     // WS2812 灯珠数据引脚
#define PIN_RGB_EN     10    // WS2812 灯珠供电使能
#define RGB_NUM        22    // WS2812 灯珠数量

Adafruit_NeoPixel strip(RGB_NUM, PIN_RGB, NEO_GRB + NEO_KHZ800);
StrandtestController strandtest(strip);

///
void ButtonClick(void *oneButton)
{
  Serial.print(((OneButton *)oneButton)->getPressedMs());
  Serial.println("\t - Click()");
    digitalWrite(PIN_LED, HIGH);
    delay(500);
    digitalWrite(PIN_LED, LOW);
}

// this function will be called when the button started long pressed.
void LongPressStart(void *oneButton)
{
  Serial.print(((OneButton *)oneButton)->getPressedMs());
  Serial.println("\t - LongPressStart()");
}

// this function will be called when the button is released.
void LongPressStop(void *oneButton)
{
  Serial.print(((OneButton *)oneButton)->getPressedMs());
  Serial.println("\t - LongPressStop()\n");
}

// this function will be called when the button is held down.
void DuringLongPress(void *oneButton)
{
  Serial.print(((OneButton *)oneButton)->getPressedMs());
  Serial.println("\t - DuringLongPress()");
}



void setup() {
  // PIN INIT
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_RGB_EN, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_RGB_EN, HIGH);
  
  strandtest.begin();
  
  Serial.begin(115200);
  Serial.println("\nOneButton Example.");
  Serial.println("Please press and hold the button for a few seconds.");

  // link functions to be called on events.
  main_button.attachClick(ButtonClick, &main_button);
  main_button.attachLongPressStart(LongPressStart, &main_button);
  main_button.attachDuringLongPress(DuringLongPress, &main_button);
  main_button.attachLongPressStop(LongPressStop, &main_button);

  main_button.setLongPressIntervalMs(1000);
}

void loop() {
  strandtest.update();
  // keep watching the push button:
  main_button.tick();
}
