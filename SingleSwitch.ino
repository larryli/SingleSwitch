// 单键 WiFi 开关
//
// author: Larry Li
// email: larryli@qq.com
// url: https://github.com/larryli/SingleSwitch

#include <ESP8266WiFi.h> // https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include <ESPAsyncWebServer.h> // https://github.com/me-no-dev/ESPAsyncWebServer

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
  led_event(e);
  button_event(e);
}

// Relay 继电器

// 继电器设置
static void relay_setup()
{
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH);
}

// 处理继电器相关事件
static void relay_event(Event e)
{
  if (e == EVENT_TOGGLE) {
    Serial.println(F("Relay toggle"));
    e = (digitalRead(RELAY) == LOW) ? EVENT_OFF : EVENT_ON;
  }
  switch (e) {
    case EVENT_ON:
      digitalWrite(RELAY, LOW);
      Serial.println(F("Relay on"));
      break;
    case EVENT_OFF:
      digitalWrite(RELAY, HIGH);
      Serial.println(F("Relay off"));
      break;
  }
}

// Touch 触摸按键

// 触摸按键配置
static void touch_setup()
{
  pinMode(TOUCH, INPUT);
  // 按下触发继电器切换事件
  attachInterrupt(digitalPinToInterrupt(TOUCH), [](){
    Serial.println(F("Touch pushed"));
    event(EVENT_TOGGLE);
  }, RISING);
}

// Led 指示灯显示

static Ticker led_ticker;

// 指示灯配置
static void led_setup()
{
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, HIGH); // 黄灯在启动时会预先点亮
}

// 处理指示灯事件
static void led_event(const Event e)
{
  switch (e) {
    case EVENT_CONNECTING: // 正在联网，绿灯闪
      digitalWrite(YELLOW_LED, LOW);
      // 切换指示灯亮灭
      led_ticker.attach_ms(500, [](){
        digitalWrite(GREEN_LED, (digitalRead(GREEN_LED) == LOW) ? HIGH : LOW);
      });
      return;
    case EVENT_CONNECTED: // 联网正常，绿灯亮
      led_ticker.detach();
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
      return;
    case EVENT_DISCONNECTED: // 联网错误，黄灯亮
      led_ticker.detach();
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(YELLOW_LED, HIGH);
      return;
    case EVENT_CONFIG: // 配网状态，黄灯闪
      digitalWrite(GREEN_LED, LOW);
      // 切换指示灯亮灭
      led_ticker.attach_ms(500, [](){
        digitalWrite(YELLOW_LED, (digitalRead(YELLOW_LED) == LOW) ? HIGH : LOW);
      });
      return;
  }
}

// Config button 配网按键

static bool button_configging = false;

// 配网按键配置
static void button_setup()
{
  pinMode(CONFIG_BUTTON, INPUT);
  // 按键处理，去抖动，根据是否在配网开启配网或取消配网
  attachInterrupt(digitalPinToInterrupt(CONFIG_BUTTON), [](){
    static uint32_t last = 0;
    uint32_t now = millis();

    if (now > last + 200) {
      last = now;
      Serial.println(F("Set button pushed"));
      if (button_configging) {
        event(EVENT_CANCEL);
      } else {
        event(EVENT_CONFIG);
      }
    }
  }, RISING);
}

// 配网按键事件处理，判断是否在配网
static void button_event(const Event e)
{
  switch (e) {
    case EVENT_CONFIG:
      button_configging = true;
      break;
    case EVENT_RECEIVED:
    case EVENT_CANCEL:
      button_configging = false;
      break;
  }
}

// WiFi 网络处理

// 网络状态
typedef enum {
  WIFI_CONNECTING,
  WIFI_CONNECTED,
  WIFI_DISCONNECTED,
  WIFI_CONFIG,
} WifiState;

static WifiState wifi_state; // 是否在配网
static uint8_t wifi_retry;
static String mdns_name;

#define WIFI_MAXRETRY 3

// WiFi 配置
static void wifi_setup()
{
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.hostname(mdns_name);
  wifi_retry = 0;
  wifi_state = WIFI_CONNECTING;
  if (WiFi.SSID() == "") {
    event(EVENT_CONFIG); // 没有配置，开始配置
  } else {
    event(EVENT_CONNECTING); // 有配置，连接
  }
}

// WiFi 循环
static bool wifi_loop()
{
  switch (wifi_state) {
    case WIFI_CONNECTING:
      switch (WiFi.status()) {
        case WL_CONNECTED:
          event(EVENT_CONNECTED);
          return true;
        case WL_CONNECT_FAILED:
          event(EVENT_FAILED);
          return true;
        case WL_NO_SSID_AVAIL:
          event(EVENT_DISCONNECTED);
          return true;
      }
      break;
    case WIFI_CONNECTED:
      if (WiFi.status() == WL_DISCONNECTED) {
        event(EVENT_DISCONNECTED);
        return true;
      }
      break;
    case WIFI_DISCONNECTED:
      if (WiFi.status() == WL_CONNECTED) {
        event(EVENT_CONNECTED);
        return true;
      }
      break;
    case WIFI_CONFIG:
      if (WiFi.smartConfigDone()) {
        event(EVENT_RECEIVED);
        return true;
      }
      break;
  }
  return false;
}

// WiFi 事件处理
static void wifi_event(const Event e)
{
  switch (e) {
    case EVENT_CONNECTING: // 连接已配置的 WiFi
      Serial.print(F("WiFi connect: "));
      Serial.println(WiFi.SSID());
      wifi_state = WIFI_CONNECTING;
      WiFi.begin();
      return;
    case EVENT_CONFIG: // 配置 WiFi 信息
      Serial.println(F("WiFi config start"));
      wifi_state = WIFI_CONFIG;
      wifi_retry = WIFI_MAXRETRY;
      WiFi.persistent(true);
      WiFi.beginSmartConfig();
      return;
    case EVENT_RECEIVED: // 配置 WiFi 信息成功
      Serial.println(F("WiFi config received"));
      WiFi.persistent(false);
      event(EVENT_CONNECTING); // 连网
      return;
    case EVENT_CANCEL: // 取消配网
      Serial.println(F("WiFi config cancel"));
      WiFi.stopSmartConfig();
      WiFi.persistent(false);
      event(EVENT_CONNECTING); // 连网
      return;
    case EVENT_FAILED: // 密码错误，WiFi 连接失败
      Serial.println(F("WiFi connect failed"));
      if (++wifi_retry < WIFI_MAXRETRY) {
        event(EVENT_CONNECTING); // 连网
        return;
      }
      event(EVENT_CONFIG); // 配网
      return;
    case EVENT_DISCONNECTED: // WiFi 未连接
      Serial.println(F("WiFi disconnected"));
      wifi_state = WIFI_DISCONNECTED;
      return;
    case EVENT_CONNECTED: // Wifi 连接成功
      Serial.print(F("WiFi connected: "));
      Serial.println(WiFi.localIP());
      wifi_state = WIFI_CONNECTED;
      wifi_retry = 0;
      return;
  }
}

// mDNS 服务

static bool mdns_state; // MDNS 是否已启动

// mDNS 配置，使用预定义名称和 mac 地址后三位命名设备
static void mdns_setup()
{
  String mac = WiFi.macAddress();

  mdns_name = "SWITCH_" + mac.substring(9, 11) + mac.substring(12, 14) + mac.substring(15, 17);
  mdns_name.toUpperCase();
  mdns_state = false;
}

// 网络连接成功时，开启 mDNS 服务，注册设备名和指定服务
static void mdns_event(const Event e)
{
  if (e != EVENT_CONNECTED) {
    return;
  }
  if (mdns_state) {
    MDNS.update();
    return;
  }
  mdns_state = MDNS.begin(mdns_name.c_str());
  if (!mdns_state) {
    Serial.println(F("mDNS error"));
    return;
  }
  MDNS.addService(F("http"), F("tcp"), 80);
  MDNS.addService(F("SWITCH_HTTP"), F("tcp"), 80);
  Serial.print(F("mDNS success: "));
  Serial.println(mdns_name);
}

// HTTP 服务

static AsyncWebServer server(80);
//static AsyncEventSource es("/events");

// HTTP 服务配置
void server_setup()
{
  server.on("/", HTTP_GET, server_root);
  server.onNotFound([](AsyncWebServerRequest * request) {
    request->send(404);
  });
  server.begin();
  Serial.println(F("Server start"));
}

static void server_root(AsyncWebServerRequest *request)
{
  request->send(200, F("text/plain"), F("Hello Switch"));
}
