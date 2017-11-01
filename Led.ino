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
