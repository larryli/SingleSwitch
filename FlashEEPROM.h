// Flash 多扇区循环保存
//
// author: Larry Li
// email: larryli@qq.com

#ifndef FLASH_EEPROM_h
#define FLASH_EEPROM_h

#include <stddef.h>
#include <stdint.h>
#include <string.h>

class FlashEEPROM {
public:
  void begin(void *data, size_t data_size, int sector_start, size_t sector_size);
  void begin(void *data, size_t data_size, int sector_start) {
    begin(data, data_size, sector_start, 2);
  }

  void update(void *data);

protected:
  uint32_t _sector_start;
  uint32_t _sector_end;
  uint32_t _sector;
  uint16_t _offset;
  size_t _data_size;
  size_t _buf_size;
  uint8_t *_buf;

  uint32_t crc32();
  bool empty();
  void write(void *data, bool erase);
};

#endif
