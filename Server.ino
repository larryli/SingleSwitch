// HTTP 服务

static AsyncWebServer server(80);
//static AsyncEventSource es("/events");

// HTTP 服务配置
void server_setup()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, F("text/plain"), F("Hello Switch"));
  });
  server.on("/api/switch", HTTP_GET, server_get_switch);
  server.on("/api/switch", HTTP_POST | HTTP_PUT, server_put_switch);
  server.on("/api/setting", HTTP_GET, server_get_setting);
  server.on("/api/setting", HTTP_POST | HTTP_PUT, server_put_setting);
  server.on("/api/task", HTTP_GET, server_get_task);
  server.on("/api/task", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasArg(F("id"))) {
      if (request->hasArg(F("_method")) && request->arg(F("_method")).equalsIgnoreCase(F("DELETE"))) {
        server_delete_task(request);
      } else {
        server_put_task(request);
      }
    } else {
      server_post_task(request);
    }
  });
  server.on("/api/task", HTTP_PUT, server_put_task); // 与 POST /api/task?id=0 相同
  server.on("/api/task", HTTP_DELETE, server_delete_task); // 与 POST /api/task?id=0&_method=delete 相同
  server.onNotFound([](AsyncWebServerRequest * request) {
    request->send(404);
  });
  server.begin();
  Serial.println(F("Server start"));
}

static void server_get_switch(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream(F("text/json"));
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  
  root["ok"] = 1;
  root["switch"] = (bool)(digitalRead(RELAY) == LOW);
  root.printTo(*response);
  request->send(response);
}

static void server_put_switch(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream(F("text/json"));
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  if (request->hasArg("switch")) {
    String arg = request->arg("switch");

    event((arg == "0") ? EVENT_RELAY_ON : EVENT_RELAY_OFF);
  } else {
    event(EVENT_RELAY_TOGGLE);
  }
  root["ok"] = 1;
  root.printTo(*response);
  request->send(response);
}

static void server_get_setting(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream(F("text/json"));
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  root["ok"] = 1;
  root["device_name"] = config.device_name;
  root["relay_keep"] = config.relay_keep;
  root["relay_auto_off"] = config.relay_auto_off;
  root["led_auto_off"] = config.led_auto_off;
  root["time_server1"] = config.time_server1;
  root["time_server2"] = config.time_server2;
  root["time_zone"] = config.time_zone;
  root.printTo(*response);
  request->send(response);
}

static void server_put_setting(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream(F("text/json"));
  String device_name = request->arg(F("device_name"));
  String relay_keep = request->arg(F("relay_keep"));
  String relay_auto_off = request->arg(F("relay_auto_off"));
  String led_auto_off = request->arg(F("led_auto_off"));
  String time_server1 = request->arg(F("time_server1"));
  String time_server2 = request->arg("time_server2");
  String time_zone = request->arg("time_zone");
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  if (device_name == nullptr) {
    root["message"] = F("没有填写设备名称");
  } else if (relay_keep == nullptr) {
    root["message"] = F("没有填写断电记忆参数");
  } else if (relay_auto_off == nullptr) {
    root["message"] = F("没有填写打开后自动关闭参数");
  } else if (led_auto_off == nullptr) {
    root["message"] = F("没有填写指示灯自动熄灭参数");
  } else if (time_server1 == nullptr) {
    root["message"] = F("没有填写时间服务器一地址");
  } else if (time_server2 == nullptr) {
    root["message"] = F("没有填写时间服务器二地址");
  } else if (time_zone == nullptr) {
    root["message"] = F("没有提供时区参数");
  } else {
    switch (config_update(device_name, relay_keep.toInt(), relay_auto_off.toInt(), led_auto_off.toInt(), time_server1, time_server2, time_zone.toInt())) {
      case 0:
        root["ok"] = 1;
        root.printTo(*response);
        request->send(response);
        return;
      case 1:
        root["message"] = F("设备名称太长");
        break;
      case 2:
        root["message"] = F("设备名称不符合要求，只能是英文数字和下划线");
        break;
      case 3:
        root["message"] = F("设置开关打开后自动关闭不能同时开启断电记忆");
        break;
      case 4:
        root["message"] = F("开关打开后自动关闭时间设置必须在 255 秒以内，并不能为负数");
        break;
      case 5:
        root["message"] = F("指示灯自动关闭时间设置必须在 255 秒以内，并不能为负数");
        break;
      case 10:
        root["message"] = F("时间服务器一地址太长");
        break;
      case 11:
        root["message"] = F("时间服务器一地址不符合要求，只能是英文数字和点");
        break;
      case 12:
        root["message"] = F("时间服务器二地址太长");
        break;
      case 13:
        root["message"] = F("时间服务器二地址不符合要求，只能是英文数字和点");
        break;
      case 14:
        root["message"] = F("时区参数错误，必须在 UTC -12:00 到 UTC 14:00 范围内");
        break;
    }
  }
  root["ok"] = 0;
  root.printTo(*response);
  response->setCode(422);
  request->send(response);
}

static void server_get_task(AsyncWebServerRequest *request)
{
}

static void server_post_task(AsyncWebServerRequest *request)
{
}

static void server_put_task(AsyncWebServerRequest *request)
{
}

static void server_delete_task(AsyncWebServerRequest *request)
{
}
