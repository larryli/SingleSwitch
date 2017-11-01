// SNTP 时间

static Ticker time_ticker;

static void time_tick()
{
  long t = (long)time(nullptr);

  if (t > 0) {
    static long last = 0;
    long m = t / 60;

    if (m != last) {
      // 一分钟只处理一次
      // 有可能上次是 08:01:00，设定 60 秒中断理应是 08:02:00
      // 但这期间经过对时后，中断后取到的却是 08:01:59
      // 此时应抛弃修正偏差的中断时间数据
      // 注：如果取到的是 08:02:01 则不影响处理
      Serial.print(F("Time: "));
      Serial.print(t);
      Serial.print(" ");
      Serial.println(ctime(&t));
      last = m;
    }
    // 每分钟修正本地与网络时间的偏差
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
  configTime(8 * 3600, 0, "cn.ntp.org.cn", "pool.ntp.org", "time.nist.gov");
  time_ticker.attach_ms(1000, time_tick);
}
