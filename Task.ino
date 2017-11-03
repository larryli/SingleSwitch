struct Task {
  uint8_t day; // 最高位保留，从最低位开始依次表示周日、一到六，0x00 为单次任务，0x7F 为每天，0x3E 为工作日，0x41 为休息日
  uint8_t hour; // 小时，0 到 23
  uint8_t minute; // 分钟，0 到 59
  uint8_t state; // 高五位保留，最低位为任务是否有效，中间位为任务是否启用（单次任务执行后自动禁用），高位为任务是打开还是关闭
};

#define TASK_NTASK 31

static Task tasks[TASK_NTASK];
static FlashEEPROM task_eeprom;

static void task_setup()
{
  memset(tasks, 0, sizeof(Task) * TASK_NTASK);
  task_eeprom.begin(tasks, sizeof(Task) * TASK_NTASK, EEPROM_TASK);
}

static void task_event(const Event e)
{
  switch (e) {
    case EVENT_TASK_UPDATE:
      Serial.println(F("Task update"));
      task_eeprom.update(tasks);
      break;
  }
}

static void task_time(const struct tm *tm)
{
  for (int n = 0; n < TASK_NTASK; n++) {
    if ((tasks[n].state & 3) && tasks[n].hour == tm->tm_hour && tasks[n].minute == tm->tm_min) { // 有效并启用的数据，且时间相同
      if (tasks[n].day == 0 || (tasks[n].day & (1 << tm->tm_wday))) { // 单次或星期相同
        Serial.print(F("Task #"));
        Serial.print(n);
        Serial.println(F(" run"));
        event(tasks[n].state & 4 ? EVENT_RELAY_ON : EVENT_RELAY_OFF); // 开或关
        if (tasks[n].day == 0) { // 单次自动禁用
          Serial.print(F("Task #"));
          Serial.print(n);
          Serial.println(F(" disabled"));
          tasks[n].state = tasks[n].state & 5;
          event(EVENT_TASK_UPDATE);
        }
        break; // 只处理一个，后续忽略
      }
    }
  }
}

static uint8_t task_insert(int day, uint8_t hour, uint8_t minute, bool state, bool relay)
{
  uint8_t i = TASK_NTASK;

  day &= 0x7F; // 兼容性处理
  if (hour < 0 || hour > 23) {
    return 1; // 小时不对
  }
  if (minute < 0 || minute > 59) {
    return 2; // 分钟不对
  }
  for (uint8_t n = 0; n < TASK_NTASK; n++) {
    if (tasks[n].state & 1) { // 有效数据
      if (tasks[n].hour == hour && tasks[n].minute == minute) { // 时间相同
        if (day == 0 || tasks[n].day == 0 || (tasks[n].day & day)) { // 天数相同
          return 3; // 已存在相同时间设置的任务
        }
      }
    } else if (i == TASK_NTASK) {
      i = n;
    }
  }
  if (i >= TASK_NTASK) {
    return 4; // 任务数已满
  }
  tasks[i].day = day;
  tasks[i].hour = hour;
  tasks[i].minute = minute;
  tasks[i].state = (relay << 2) | (state < 1) | 1;
  event(EVENT_TASK_UPDATE);
  return 0;
}

static uint8_t task_update(int i, int day, int hour, int minute, bool state, bool relay)
{
  if (i < 0 || i >= TASK_NTASK || !(tasks[i].state & 1)) {
    return 5; // 任务号不存在
  }
  day &= 0x7F; // 兼容性处理
  if (hour < 0 || hour > 23) {
    return 1; // 小时不对
  }
  if (minute < 0 || minute > 59) {
    return 2; // 分钟不对
  }
  for (uint8_t n = 0; n < TASK_NTASK; n++) {
    if (n != i && tasks[n].state & 1 && tasks[n].hour == hour && tasks[n].minute == minute) { // 有效数据并排除自己，且时间相同
      if (day == 0 || tasks[n].day == 0 || (tasks[n].day & day)) { // 天数相同
        return 3; // 已存在相同时间设置的任务
      }
    }
  }
  tasks[i].day = day;
  tasks[i].hour = hour;
  tasks[i].minute = minute;
  tasks[i].state = (relay << 2) | (state < 1) | 1;
  event(EVENT_TASK_UPDATE);
  return 0;
}

static uint8_t task_delete(int i)
{
  if (i < 0 || i >= TASK_NTASK || !(tasks[i].state & 1)) {
    return 5; // 任务号不存在
  }
  if (i < TASK_NTASK - 1) {
    memcpy(&tasks[i + 1], &tasks[i], sizeof(Task) * (TASK_NTASK - 1 - i));
  }
  memset(&tasks[TASK_NTASK - 1], 0, sizeof(Task));
  event(EVENT_TASK_UPDATE);
  return 0;
}
