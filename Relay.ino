// Relay 继电器

// 继电器设置
static void relay_setup()
{
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH);
}

// 处理继电器相关事件
static void relay_event(Event e)
{
  if (e == EVENT_TOGGLE) {
    Serial.println(F("Relay toggle"));
    e = (digitalRead(RELAY) == LOW) ? EVENT_OFF : EVENT_ON;
  }
  switch (e) {
    case EVENT_ON:
      digitalWrite(RELAY, LOW);
      Serial.println(F("Relay on"));
      break;
    case EVENT_OFF:
      digitalWrite(RELAY, HIGH);
      Serial.println(F("Relay off"));
      break;
  }
}
