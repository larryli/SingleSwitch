// Config button 配网按键

static bool button_configging = false;

// 配网按键配置
static void button_setup()
{
  pinMode(CONFIG_BUTTON, INPUT_PULLUP);
  // 按键处理，去抖动，根据是否在配网开启配网或取消配网；处理 millis 翻转
  attachInterrupt(digitalPinToInterrupt(CONFIG_BUTTON), [](){
    static uint32_t last = 0;
    uint32_t now = millis();

    if (last > now || now > last + 200) {
      last = now;
      Serial.println(F("Set button pushed"));
      if (button_configging) {
        event(EVENT_SMART_CANCEL);
      } else {
        event(EVENT_SMART_CONFIG);
      }
    }
  }, FALLING);
}

// 配网按键事件处理，判断是否在配网
static void button_event(const Event e)
{
  switch (e) {
    case EVENT_SMART_CONFIG:
      button_configging = true;
      break;
    case EVENT_SMART_RECEIVED:
    case EVENT_SMART_CANCEL:
      button_configging = false;
      break;
  }
}
