// Led 指示灯显示

static Ticker led_ticker;
static bool led_state;

// 指示灯配置
static void led_setup()
{
  led_state = false;
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, HIGH); // 绿灯在启动时会预先点亮
}

// 处理指示灯事件
static void led_event(const Event e)
{
  switch (e) {
    case EVENT_WIFI_CONNECTING: // 正在联网，绿灯闪
      led_state = false;
      digitalWrite(YELLOW_LED, LOW);
      // 切换指示灯亮灭
      led_ticker.attach_ms(500, [](){
        digitalWrite(GREEN_LED, (digitalRead(GREEN_LED) == LOW) ? HIGH : LOW);
      });
      break;
    case EVENT_WIFI_CONNECTED: // 联网正常，绿灯亮
      led_state = true;
      led_ticker.detach();
      digitalWrite(YELLOW_LED, LOW);
      led_green();
      break;
    case EVENT_WIFI_DISCONNECTED: // 联网错误，黄灯亮
      led_state = false;
      led_ticker.detach();
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(YELLOW_LED, HIGH);
      break;
    case EVENT_SMART_CONFIG: // 配网状态，黄灯闪
      led_state = false;
      digitalWrite(GREEN_LED, LOW);
      // 切换指示灯亮灭
      led_ticker.attach_ms(500, [](){
        digitalWrite(YELLOW_LED, (digitalRead(YELLOW_LED) == LOW) ? HIGH : LOW);
      });
      break;
    case EVENT_RELAY_UPDATE: // 反馈操作，绿灯闪一下；只在联网正常时处理
      if (led_state) {
        digitalWrite(GREEN_LED, LOW);
        led_ticker.once_ms(500, [](){
          led_green();
        });
      }
      break;
  }
}

static void led_green()
{
  digitalWrite(GREEN_LED, HIGH);
  if (config.led_auto_off > 0) {
    led_ticker.once(config.led_auto_off, [](){
      digitalWrite(GREEN_LED, LOW);
    });
  }
}
