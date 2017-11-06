// HTTP 服务 API 接口代码

//static AsyncEventSource api_event("/api/event");

static void api_setup()
{
  api_on(&server, "/api/switch", HTTP_GET, api_get_switch);
  api_on(&server, "/api/switch", HTTP_POST | HTTP_PUT, api_put_switch);
  api_on(&server, "/api/setting", HTTP_GET, api_get_setting);
  api_on(&server, "/api/setting", HTTP_POST | HTTP_PUT, api_put_setting);
  api_on(&server, "/api/task", HTTP_GET, api_get_task);
  api_on(&server, "/api/task", HTTP_POST, [](AsyncWebServerRequest * request, AsyncResponseStream *response, JsonObject &root) {
    if (request->hasArg(F("id"))) {
      if (request->hasArg(F("_method")) && request->arg(F("_method")).equalsIgnoreCase(F("DELETE"))) {
        api_delete_task(request, response, root);
      } else {
        api_put_task(request, response, root);
      }
    } else {
      api_post_task(request, response, root);
    }
  });
  api_on(&server, "/api/task", HTTP_PUT, api_put_task); // 与 POST /api/task?id=0 相同
  api_on(&server, "/api/task", HTTP_DELETE, api_delete_task); // 与 POST /api/task?id=0&_method=delete 相同
}

static AsyncCallbackWebHandler& api_on(AsyncWebServer *server, const char* uri, WebRequestMethodComposite method, ApiHandler handler)
{
  return server->on(uri, method, [&](AsyncWebServerRequest * request) {
    AsyncResponseStream *response = request->beginResponseStream(F("text/json"));
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    handler(request, response, root);
    if (root["ok"] == 0) {
      response->setCode(422);
    }
    root.printTo(*response);
    request->send(response);
  });
}

static void api_get_switch(AsyncWebServerRequest *request, AsyncResponseStream *response, JsonObject &root)
{
  root["ok"] = 1;
  root["switch"] = (bool)(digitalRead(RELAY) == LOW);
}

static void api_put_switch(AsyncWebServerRequest *request, AsyncResponseStream *response, JsonObject &root)
{
  if (request->hasArg("switch")) {
    String arg = request->arg("switch");

    event((arg == "0") ? EVENT_RELAY_ON : EVENT_RELAY_OFF);
  } else {
    event(EVENT_RELAY_TOGGLE);
  }
  root["ok"] = 1;
}

static void api_get_setting(AsyncWebServerRequest *request, AsyncResponseStream *response, JsonObject &root)
{
  root["ok"] = 1;
  root["device_name"] = config.device_name;
  root["relay_keep"] = config.relay_keep;
  root["relay_auto_off"] = config.relay_auto_off;
  root["led_auto_off"] = config.led_auto_off;
  root["time_server1"] = config.time_server1;
  root["time_server2"] = config.time_server2;
  root["time_zone"] = config.time_zone;
}

static void api_put_setting(AsyncWebServerRequest *request, AsyncResponseStream *response, JsonObject &root)
{
  String device_name = request->arg(F("device_name"));
  String relay_keep = request->arg(F("relay_keep"));
  String relay_auto_off = request->arg(F("relay_auto_off"));
  String led_auto_off = request->arg(F("led_auto_off"));
  String time_server1 = request->arg(F("time_server1"));
  String time_server2 = request->arg("time_server2");
  String time_zone = request->arg("time_zone");

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
      default:
        root["message"] = F("未知错误，请更新固件");
        break;
    }
  }
  root["ok"] = 0;
}

static void api_get_task(AsyncWebServerRequest *request, AsyncResponseStream *response, JsonObject &root)
{
}

static void api_post_task(AsyncWebServerRequest *request, AsyncResponseStream *response, JsonObject &root)
{
  String day = request->arg(F("day"));
  String hour = request->arg(F("hour"));
  String minute = request->arg(F("minute"));
  String state = request->arg(F("state"));
  String relay = request->arg(F("relay"));

  if (day == nullptr) {
    root["message"] = F("没有选择天");
  } else if (hour == nullptr) {
    root["message"] = F("没有选择小时");
  } else if (minute == nullptr) {
    root["message"] = F("没有选择分钟");
  } else if (state == nullptr) {
    root["message"] = F("没有设置状态");
  } else if (relay == nullptr) {
    root["message"] = F("没有选择开关动作");
  } else {
    switch (task_insert(day.toInt(), hour.toInt(), minute.toInt(), state.toInt(), relay.toInt())) {
      case 0:
        root["ok"] = 1;
        return;
      case 1:
        root["message"] = F("小时不对");
        break;
      case 2:
        root["message"] = F("分钟不对");
        break;
      case 3:
        root["message"] = F("已存在相同时间设置的任务");
        break;
      case 4:
        root["message"] = F("任务数已满");
        break;
      case 5:
        root["message"] = F("任务号不存在");
        break;
      default:
        root["message"] = F("未知错误，请更新固件");
        break;
    }
  }
  root["ok"] = 0;
}

static void api_put_task(AsyncWebServerRequest *request, AsyncResponseStream *response, JsonObject &root)
{
  String id = request->arg(F("id"));
  String day = request->arg(F("day"));
  String hour = request->arg(F("hour"));
  String minute = request->arg(F("minute"));
  String state = request->arg(F("state"));
  String relay = request->arg(F("relay"));

  if (id == nullptr) {
    root["message"] = F("没有提供任务 ID");
  } else if (day == nullptr) {
    root["message"] = F("没有选择天");
  } else if (hour == nullptr) {
    root["message"] = F("没有选择小时");
  } else if (minute == nullptr) {
    root["message"] = F("没有选择分钟");
  } else if (state == nullptr) {
    root["message"] = F("没有设置状态");
  } else if (relay == nullptr) {
    root["message"] = F("没有选择开关动作");
  } else {
    switch (task_update(id.toInt(), day.toInt(), hour.toInt(), minute.toInt(), state.toInt(), relay.toInt())) {
      case 0:
        root["ok"] = 1;
        return;
      case 1:
        root["message"] = F("小时不对");
        break;
      case 2:
        root["message"] = F("分钟不对");
        break;
      case 3:
        root["message"] = F("已存在相同时间设置的任务");
        break;
      case 5:
        root["message"] = F("任务号不存在");
        break;
      default:
        root["message"] = F("未知错误，请更新固件");
        break;
    }
  }
  root["ok"] = 0;
}

static void api_delete_task(AsyncWebServerRequest *request, AsyncResponseStream *response, JsonObject &root)
{
  String id = request->arg(F("id"));

  if (id == nullptr) {
    root["message"] = F("没有提供任务 ID");
  } else {
    switch (task_delete(id.toInt())) {
      case 0:
        root["ok"] = 1;
        return;
      case 5:
        root["message"] = F("任务号不存在");
        break;
      default:
        root["message"] = F("未知错误，请更新固件");
        break;
    }
  }
  root["ok"] = 0;
}
