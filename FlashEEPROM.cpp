// Flash 多扇区循环保存
//
// author: Larry Li
// email: larryli@qq.com

#include "Arduino.h"
#include "FlashEEPROM.h"

extern "C" {
#include "c_types.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "spi_flash.h"
}

extern "C" uint32_t _SPIFFS_end;

void FlashEEPROM::begin(void *data, size_t data_size, int sector_start, size_t sector_size)
{
  if (sector_start < 0) {
    // 使用负数将从 FS 尾部向前偏移扇区
    _sector_start = ((((uint32_t)&_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE)) + sector_start;
  } else {
    _sector_start = sector_start;
  }
  _sector_end = _sector_start + sector_size - 1;
  _data_size = data_size;
  _buf_size = ((_data_size + 3) & (~3)) + 4 + 120;
  _sector = _sector_start;
  _offset = 0;
  _buf = new uint8_t[_buf_size];
  while (_sector <= _sector_end) {
    while(_offset < SPI_FLASH_SEC_SIZE) {
      if (spi_flash_read(_sector * SPI_FLASH_SEC_SIZE + _offset, reinterpret_cast<uint32_t*>(_buf), _buf_size) == SPI_FLASH_RESULT_OK) {
        if (empty()) {
          // 找到可用块，结束
          free(_buf);
          return;
        } else {
          uint32_t *crc = (uint32_t *)&(_buf[_buf_size - 4]);

          if (crc32() == *crc) {
            // 找到验证的数据，复制
            memcpy(data, _buf, _data_size);
          }
        }
      }
      _offset += _buf_size;
    }
    _sector++;
    _offset = 0;
  }
  // 已使用所有块，擦除首扇区，并回写数据
  _sector = _sector_start;
  _offset = 0;
  write(data, true);
}

void FlashEEPROM::update(void *data)
{
  _buf = new uint8_t[_buf_size];
  while (_sector <= _sector_end) {
    while(_offset < SPI_FLASH_SEC_SIZE) {
      if (spi_flash_read(_sector * SPI_FLASH_SEC_SIZE + _offset, reinterpret_cast<uint32_t*>(_buf), _buf_size) == SPI_FLASH_RESULT_OK) {
        if (empty()) {
          // 找到可用块，写数据
          write(data, false);
          return;
        } else if (_offset == 0) {
          // 换扇区，第一个块不可用，擦写并写数据
          write(data, true);
          return;
        }
      }
      _offset += _buf_size;
    }
    _sector++;
    _offset = 0;
  }
  // 已使用所有块，擦除首扇区，并写数据
  _sector = _sector_start;
  _offset = 0;
  write(data, true);
}

uint32_t FlashEEPROM::crc32()
{
  const uint8_t *data = _buf;
  size_t length = _data_size;
  uint32_t crc = 0xFFFFFFFF;

  while (length--) {
    uint8_t c = *data++;
    for (uint32_t i = 0x80; i > 0; i >>= 1) {
      bool bit = crc & 0x80000000;
      if (c & i) {
        bit = !bit;
      }
      crc <<= 1;
      if (bit) {
        crc ^= 0x04C11DB7;
      }
    }
  }
  return crc;
}

bool FlashEEPROM::empty()
{
  const uint8_t *data = _buf;
  size_t length = _buf_size;

  while (length--) {
    if (*data++ != 0xFF) {
      return false;
    }
  }
  return true;
}

void FlashEEPROM::write(void *data, bool erase)
{
  bool next = false;
  uint32_t *crc = (uint32_t *)&(_buf[_buf_size - 4]);

  if (!erase && (_offset + _buf_size) >= SPI_FLASH_SEC_SIZE && _sector < _sector_end) {
    // 下一个块跨扇区，而且不是回到第一个扇区
    if (spi_flash_read((_sector + 1) * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(_buf), _buf_size) == SPI_FLASH_RESULT_OK) {
      next = !empty();
    }
  }
  memset(_buf, 0, _buf_size);
  memcpy(_buf, data, _data_size);
  *crc = crc32();
  noInterrupts();
  if (erase) {
    spi_flash_erase_sector(_sector);
  } else if (next) {
    spi_flash_erase_sector(_sector + 1);
  }
  spi_flash_write(_sector * SPI_FLASH_SEC_SIZE + _offset, reinterpret_cast<uint32_t*>(_buf), _buf_size);
  interrupts();
  free(_buf);
  _offset += _buf_size;
}

