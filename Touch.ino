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


