// mDNS 服务

static bool mdns_state;

static void mdns_setup()
{
  mdns_state = false;
  MDNS.addService(F("http"), F("tcp"), 80);
  MDNS.addService(F("SISW_HTTP"), F("tcp"), 80);
}

// 网络连接成功时，开启 mDNS 服务，注册设备名和指定服务
static void mdns_event(const Event e)
{
  switch (e) {
    case EVENT_WIFI_CONNECTED:
      if (mdns_state) {
        return;
      }
      mdns_state = MDNS.begin(config.mdns_name);
      if (!mdns_state) {
        Serial.println(F("mDNS error"));
        return;
      }
      Serial.print(F("mDNS success: "));
      Serial.println(config.mdns_name);
      break;
    case EVENT_CONFIG_UPDATE:
      mdns_state = MDNS.begin(config.mdns_name);
      if (!mdns_state) {
        Serial.println(F("mDNS error"));
        return;
      }
      Serial.print(F("mDNS update: "));
      Serial.println(config.mdns_name);
      break;
  }
}
