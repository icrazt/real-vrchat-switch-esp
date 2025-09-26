#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// 按键定义
#define KEY_USER_2     2
#define KEY_USER_MAIN  4     // 主按键, 当前版本外壳唯一的操作按键
#define KEY_USER_BOOT  9     // BOOT模式切换按键，正常启动后也可作为用户按键
// LED定义
#define PIN_LED        8     // 白色指示灯引脚
#define PIN_RGB        3     // WS2812 灯珠数据引脚
#define PIN_RGB_EN     10    // WS2812 灯珠供电使能
#define RGB_NUM        22    // WS2812 灯珠数量

Adafruit_NeoPixel pixels(RGB_NUM, PIN_RGB, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 500 // Time (in milliseconds) to pause between pixels

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_RGB_EN, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_RGB_EN, HIGH);
  
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear(); // Set all pixel colors to 'off'
}

void loop() {
    for(int i=0; i<RGB_NUM; i++) { // For each pixel...

    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    // Here we're using a moderately bright green color:
    pixels.setPixelColor(i, pixels.Color(50, 0, 50));

    pixels.show();   // Send the updated pixel colors to the hardware.

    delay(DELAYVAL); // Pause before next pass through loop
  }
}
