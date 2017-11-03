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

#define WIFI_MAXRETRY 3

// WiFi 配置
static void wifi_setup()
{
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.hostname(config.device_name);
  wifi_retry = 0;
  wifi_state = WIFI_CONNECTING;
  if (WiFi.SSID() == "") {
    event(EVENT_SMART_CONFIG); // 没有配置，开始配置
  } else {
    event(EVENT_WIFI_CONNECTING); // 有配置，连接
  }
}

// WiFi 循环
static bool wifi_loop()
{
  switch (wifi_state) {
    case WIFI_CONNECTING:
      switch (WiFi.status()) {
        case WL_CONNECTED:
          event(EVENT_WIFI_CONNECTED);
          return true;
        case WL_CONNECT_FAILED:
          event(EVENT_WIFI_FAILED);
          return true;
        case WL_NO_SSID_AVAIL:
          event(EVENT_WIFI_DISCONNECTED);
          return true;
      }
      break;
    case WIFI_CONNECTED:
      if (WiFi.status() == WL_DISCONNECTED) {
        event(EVENT_WIFI_DISCONNECTED);
        return true;
      }
      break;
    case WIFI_DISCONNECTED:
      if (WiFi.status() == WL_CONNECTED) {
        event(EVENT_WIFI_CONNECTED);
        return true;
      }
      break;
    case WIFI_CONFIG:
      if (WiFi.smartConfigDone()) {
        event(EVENT_SMART_RECEIVED);
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
    case EVENT_WIFI_CONNECTING: // 连接已配置的 WiFi
      Serial.print(F("WiFi connect: "));
      Serial.println(WiFi.SSID());
      wifi_state = WIFI_CONNECTING;
      WiFi.begin();
      break;
    case EVENT_SMART_CONFIG: // 配置 WiFi 信息
      Serial.println(F("WiFi config start"));
      wifi_state = WIFI_CONFIG;
      wifi_retry = WIFI_MAXRETRY;
      WiFi.persistent(true);
      WiFi.beginSmartConfig();
      break;
    case EVENT_SMART_RECEIVED: // 配置 WiFi 信息成功
      Serial.println(F("WiFi config received"));
      WiFi.persistent(false);
      event(EVENT_WIFI_CONNECTING); // 连网
      break;
    case EVENT_SMART_CANCEL: // 取消配网
      Serial.println(F("WiFi config cancel"));
      WiFi.stopSmartConfig();
      WiFi.persistent(false);
      event(EVENT_WIFI_CONNECTING); // 连网
      break;
    case EVENT_WIFI_FAILED: // 密码错误，WiFi 连接失败
      Serial.println(F("WiFi connect failed"));
      if (++wifi_retry < WIFI_MAXRETRY) {
        event(EVENT_WIFI_CONNECTING); // 连网
        return;
      }
      event(EVENT_SMART_CONFIG); // 配网
      break;
    case EVENT_WIFI_DISCONNECTED: // WiFi 未连接
      Serial.println(F("WiFi disconnected"));
      wifi_state = WIFI_DISCONNECTED;
      break;
    case EVENT_WIFI_CONNECTED: // Wifi 连接成功
      Serial.print(F("WiFi connected: "));
      Serial.println(WiFi.localIP());
      wifi_state = WIFI_CONNECTED;
      wifi_retry = 0;
      break;
    case EVENT_CONFIG_UPDATE: // 更新配置
      WiFi.hostname(config.device_name);
      break;
  }
}
