// 单键 WiFi 开关
//
// author: Larry Li
// email: larryli@qq.com
// url: https://github.com/larryli/SingleSwitch

#include <ESP8266WiFi.h> // https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>
#include <time.h>
#include <Ticker.h>
#include <ESPAsyncWebServer.h> // https://github.com/me-no-dev/ESPAsyncWebServer

extern "C" {
  #include <gpio.h>
}

const uint8_t RELAY = 13; // 低电平触发继电器
const uint8_t TOUCH = 14; // TTP223 触发按键
const uint8_t CONFIG_BUTTON = 4; // 轻触按键，配网，取消配网
const uint8_t YELLOW_LED = 12;  // 黄色 LED，网络故障常亮，配网闪烁
const uint8_t GREEN_LED = 5; // 绿色 LED，网络正常常亮，连网闪烁

// 事件常量
typedef enum {
  EVENT_CONNECTING, EVENT_CONNECTED, EVENT_FAILED, EVENT_DISCONNECTED, // 网络连接事件
  EVENT_CONFIG, EVENT_RECEIVED, EVENT_CANCEL, // 配网事件
  EVENT_ON, EVENT_OFF, EVENT_TOGGLE, // 继电器操作事件
} Event;

// Arduino 入口
void setup() {
  Serial.begin(115200);
  Serial.println(F("\n\n"));
  Serial.println(F("Sketch start"));
  
  relay_setup();
  touch_setup();
  led_setup();
  button_setup();
  mdns_setup();
  wifi_setup();
  server_setup();

//  // 设置睡眠模式省电
//  // http://espressif.com/sites/default/files/documentation/9b-esp8266-low_power_solutions_cn.pdf
//  WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
//  gpio_pin_wakeup_enable(TOUCH, GPIO_PIN_INTR_POSEDGE);
//  gpio_pin_wakeup_enable(CONFIG_BUTTON, GPIO_PIN_INTR_POSEDGE);
}

// Arduino 主循环
void loop() {
  if (wifi_loop()) {
    return;
  }
}

// 事件处理入口
static void event(const Event e)
{
  relay_event(e);
  wifi_event(e);
  mdns_event(e);
  time_event(e);
  led_event(e);
  button_event(e);
}
