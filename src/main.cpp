#include <Arduino.h>

// 按键引脚
#define KEY_USER_2     2
#define KEY_USER_MAIN  4     //主按键, 当前版本外壳唯一的操作按键
#define KEY_USER_BOOT  9     //BOOT模式切换按键，正常启动后也可作为用户按键
// LED引脚
#define PIN_LED        8     //白色指示灯引脚
#define PIN_RGB        3     // WS2812 灯珠数据引脚
#define PIN_RGB_EN     10    // WS2812 灯珠供电使能
#define RGB_NUM        22    // WS2812 灯珠数量

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}