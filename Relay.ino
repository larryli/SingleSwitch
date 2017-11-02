// Relay 继电器

static FlashEEPROM relay_eeprom;

// 继电器设置
static void relay_setup()
{
  uint8_t v = HIGH;

  relay_eeprom.begin(&v, sizeof(v), -2);
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, v);
}

// 处理继电器相关事件
static void relay_event(Event e)
{
  uint8_t v;

  if (e == EVENT_TOGGLE) {
    Serial.println(F("Relay toggle"));
    e = (digitalRead(RELAY) == LOW) ? EVENT_OFF : EVENT_ON;
  }
  switch (e) {
    case EVENT_ON:
      v = LOW;
      digitalWrite(RELAY, v);
      relay_eeprom.update(&v);
      Serial.println(F("Relay on"));
      break;
    case EVENT_OFF:
      v = HIGH;
      digitalWrite(RELAY, v);
      relay_eeprom.update(&v);
      Serial.println(F("Relay off"));
      break;
  }
}
