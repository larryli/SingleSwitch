// SNTP 时间

static Ticker time_ticker;

static void time_tick()
{
  long t = (long)time(nullptr);

  if (t > 0) {
    static long last = 0;
    long m = t / 60;

    if (m != last) {
      Serial.print(F("Time: "));
      Serial.print(t);
      Serial.print(" ");
      Serial.println(ctime(&t));
      last = m;
    }
    time_ticker.once(60 - t % 60, time_tick);
  }
}

static void time_event(const Event e)
{
  long t = (long)time(nullptr);

  if (t > 0 || e != EVENT_CONNECTED) {
    return;
  }
  Serial.println(F("Time start"));
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  time_ticker.attach_ms(1000, time_tick);
}
