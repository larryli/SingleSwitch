/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
    union {
        struct {
            uint32_t v: 8;      /*!< Brightness/Value of the LED. 0-255 */
            uint32_t s: 8;      /*!< Saturation of the LED. 0-255 */
            uint32_t h: 9;      /*!< Hue of the LED. 0-360 */
            uint32_t i: 7;      /*!< Index of the LED. 0-126, set 127 to control all  */
        };
        uint32_t value;         /*!< IHSV value of the LED. */
    };
} led_indicator_ihsv_t;

#ifdef __cplusplus
}
#endif
