// 单键 WiFi 开关
//
// author: Larry Li
// email: larryli@qq.com
// url: https://github.com/larryli/SingleSwitch

#include <ESP8266WiFi.h> // https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>
#include <pgmspace.h>
#include <time.h>
#include <Ticker.h>
#include "JsonServer.h"
#include "FlashEEPROM.h"

extern "C" {
  #include <gpio.h>
}

const uint8_t RELAY = 13; // 低电平触发继电器
const uint8_t TOUCH = 12; // TTP223 触发按键
const uint8_t CONFIG_BUTTON = 4; // 轻触按键，配网，取消配网
const uint8_t YELLOW_LED = 5;  // 黄色 LED，网络故障常亮，配网闪烁
const uint8_t GREEN_LED = 14; // 绿色 LED，网络正常常亮，连网闪烁

// 模拟 EEPROM 保存数据的 Flash 扇区，负数表示从 FS 结尾向前倒数
#define EEPROM_RELAY -2
#define EEPROM_CONFIG -4
#define EEPROM_TASK -6

// 事件常量
typedef enum {
  EVENT_WIFI_CONNECTING, EVENT_WIFI_CONNECTED, EVENT_WIFI_FAILED, EVENT_WIFI_DISCONNECTED, // 网络连接事件
  EVENT_SMART_CONFIG, EVENT_SMART_RECEIVED, EVENT_SMART_CANCEL, // 配网事件
  EVENT_RELAY_ON, EVENT_RELAY_OFF, EVENT_RELAY_TOGGLE, EVENT_RELAY_UPDATE, // 继电器操作事件
  EVENT_CONFIG_UPDATE, // 配置更新
  EVENT_TASK_UPDATE, // 任务更新
} Event;

// Arduino 入口
void setup() {
  Serial.begin(115200);
  Serial.println(F("\r\n"));
  Serial.println(F("Sketch start"));

  config_setup();
  relay_setup();
  task_setup();
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
  mdns_event(e);
  wifi_event(e);
  config_event(e);
  time_event(e);
  led_event(e);
  button_event(e);
}
