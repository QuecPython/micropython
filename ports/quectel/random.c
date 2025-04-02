/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Daniel Campora
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "random.h"
#include "helios_rtc.h"
#include "helios_dev.h"

uint32_t qpy_rng_get(void)
{
    char imei_str[30];
    memset(imei_str, 0, sizeof(imei_str));
    int64_t ticks_us = Helios_RTC_GetTicks();
    #if MICROPY_QPY_MODULE_DSDS
    extern int _get_current_simid(void);
	int cur_simid = _get_current_simid();
    Helios_Dev_GetIMEI(imei_str, sizeof(imei_str),cur_simid);
    #else
    Helios_Dev_GetIMEI(imei_str, sizeof(imei_str),0);
    #endif
    int64_t imei_dig = 0;
    for(unsigned int i = 0; i < strlen(imei_str); i++)
    {
        imei_dig = (imei_str[i] - 0x30) + imei_dig * 10;
    }

    return (uint32_t)(((ticks_us & 0xFFFFFFFF00000000) >> 32) ^ (ticks_us & 0x00000000FFFFFFFF)) + imei_dig;
}

