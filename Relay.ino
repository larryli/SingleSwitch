// Relay 继电器

static FlashEEPROM relay_eeprom;
static Ticker relay_ticker;

// 继电器设置
static void relay_setup()
{
  uint8_t v = HIGH;

  relay_eeprom.begin(&v, sizeof(v), EEPROM_RELAY);
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, config.relay_keep ? v : HIGH);
  Serial.print(F("Relay "));
  Serial.println((digitalRead(RELAY) == LOW) ? F("on") : F("off"));
}

// 处理继电器相关事件
static void relay_event(Event e)
{
  uint8_t v;

  switch (e) {
    case EVENT_RELAY_TOGGLE:
      Serial.println(F("Relay toggle"));
      v = (digitalRead(RELAY) == LOW) ? HIGH : LOW;
      break;
    case EVENT_RELAY_ON:
      v = LOW;
      break;
    case EVENT_RELAY_OFF:
      v = HIGH;
      break;
    default:
      return;
  }
  if (v != digitalRead(RELAY)) {
    digitalWrite(RELAY, v);
    if (config.relay_keep) {
      relay_eeprom.update(&v);
    } else if (config.relay_auto_off > 0) {
      if (v == LOW) {
        relay_ticker.once(config.relay_auto_off, [](){
          digitalWrite(RELAY, HIGH);
          Serial.print(F("Relay off"));
          event(EVENT_RELAY_UPDATE);
        });
      } else {
        relay_ticker.detach();
      }
    }
    Serial.print(F("Relay "));
    Serial.println((v == LOW) ? F("on") : F("off"));
    event(EVENT_RELAY_UPDATE);
  }
}
