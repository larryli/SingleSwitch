// mDNS 服务

static bool mdns_state; // MDNS 是否已启动

// mDNS 配置，使用预定义名称和 mac 地址后三位命名设备
static void mdns_setup()
{
  String mac = WiFi.macAddress();

  mdns_name = "SISW_" + mac.substring(9, 11) + mac.substring(12, 14) + mac.substring(15, 17);
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
  MDNS.addService(F("SISW_HTTP"), F("tcp"), 80);
  Serial.print(F("mDNS success: "));
  Serial.println(mdns_name);
}