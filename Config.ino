// Config 全局配置

#define CONFIG_NCHAR 64

struct Config {
  char mdns_name[CONFIG_NCHAR];
  bool relay_keep;
  uint8_t relay_auto_off;
  uint8_t led_auto_off;
  uint8_t _reserved[53];
  char time_server1[CONFIG_NCHAR];
  char time_server2[CONFIG_NCHAR];
  int16_t time_zone;
  int16_t _time_daylight; // reserved
};

static Config config;
static FlashEEPROM config_eeprom;

static void config_setup()
{
  String mdns_name = F("SISW_"), mac = WiFi.macAddress();

  memset(&config, 0, sizeof(config));
  // mDNS 配置，使用预定义名称和 mac 地址后三位命名设备
  mdns_name = mdns_name + mac.substring(9, 11) + mac.substring(12, 14) + mac.substring(15, 17);
  mdns_name.toUpperCase();
  strcpy(config.mdns_name, mdns_name.c_str());
  config.relay_keep = true;
  strcpy_P(config.time_server1, PSTR("cn.ntp.org.cn"));
  strcpy_P(config.time_server2, PSTR("pool.ntp.org"));
  config.time_zone= 8 * 60; // UTC+8
  config_eeprom.begin(&config, sizeof(config), EEPROM_CONFIG);
  if (config.relay_keep && config.relay_auto_off > 0) {
    config.relay_keep = false; // 继电器自动关闭时不能使用断电记忆
    config_eeprom.update(&config);
  }
}

static void config_event(const Event e)
{
  if (e == EVENT_CONFIG_UPDATE) {
    Serial.println(F("Config update"));
    config_eeprom.update(&config);
  }
}

static uint8_t config_update(String mdns_name, bool relay_keep, int relay_auto_off, int led_auto_off,
  String time_server1, String time_server2, int time_zone)
{
  mdns_name.trim();
  if (mdns_name.length() >= CONFIG_NCHAR) {
    return 1; // 设备名称太长
  }
  for (uint8_t n = 0; n < mdns_name.length(); n++) {
    char c = mdns_name.charAt(n);
    if (!isAlphaNumeric(c) && c != '_' && c != '-') {
      return 2; // 设备名称不符合要求 
    }
  }
  if (relay_keep && relay_auto_off > 0) {
    return 3; // 继电器自动关闭时不能使用断电记忆
  }
  if (relay_auto_off < 0 || relay_auto_off > 255) {
    return 4; // 继电器自动关闭时参数错误
  }
  if (led_auto_off < 0 || led_auto_off > 255) {
    return 5; // LED 自动关闭时参数错误
  }
  time_server1.trim();
  if (time_server1.length() >= CONFIG_NCHAR) {
    return 10; // 时间服务器 1 名称太长
  }
  for (uint8_t n = 0; n < time_server1.length(); n++) {
    char c = time_server1.charAt(n);
    if (!isAlphaNumeric(c) && c != '.' && c != '-') {
      return 11; // 时间服务器 1 名称不符合要求 
    }
  }
  time_server2.trim();
  if (time_server2.length() >= CONFIG_NCHAR) {
    return 12; // 时间服务器 2 名称太长
  }
  for (uint8_t n = 0; n < time_server2.length(); n++) {
    char c = time_server2.charAt(n);
    if (!isAlphaNumeric(c) && c != '.' && c != '-') {
      return 13; // 时间服务器 2 名称不符合要求 
    }
  }
  // https://zh.wikipedia.org/wiki/%E6%97%B6%E5%8C%BA%E5%88%97%E8%A1%A8
  if (time_zone < -12*60 || time_zone > 14*60) {
    return 14; // 时区参数错误
  }
  mdns_name.toUpperCase();
  time_server1.toLowerCase();
  time_server2.toLowerCase();
  strcpy(config.mdns_name, mdns_name.c_str());
  config.relay_keep = relay_keep;
  config.relay_auto_off = relay_auto_off;
  config.led_auto_off = led_auto_off;
  strcpy(config.time_server1, time_server1.c_str());
  strcpy(config.time_server2, time_server2.c_str());
  config.time_zone = time_zone;
  event(EVENT_CONFIG_UPDATE);
  return 0;
}
