/*
 * Copyright (c) Quectel Wireless Solution, Co., Ltd.All Rights Reserved.
 *  
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  
 *     http://www.apache.org/licenses/LICENSE-2.0
 *  
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 ******************************************************************************
 * @file    modmachine.h
 * @author  xxx
 * @version V1.0.0
 * @date    2019/04/16
 * @brief   xxx
 ******************************************************************************
 */

#ifndef __MOD_MACHINE_H_
#define __MOD_MACHINE_H_

#include "py/obj.h"

extern const mp_obj_type_t machine_lcd_type;
extern const mp_obj_type_t machine_pin_type;
extern const mp_obj_type_t machine_uart_type;
extern const mp_obj_type_t machine_extint_type;
extern const mp_obj_type_t machine_rtc_type;
extern const mp_obj_type_t machine_timer_type;
extern const mp_obj_type_t machine_hard_i2c_type;
extern const mp_obj_type_t machine_simulation_i2c_type;
extern const mp_obj_type_t machine_hard_spi_type;
extern const mp_obj_type_t machine_wdt_type;
extern const mp_obj_type_t machine_nandflash_type;
#if defined(PLAT_ASR) || defined(PLAT_Unisoc) ||defined(PLAT_ASR_1606)
extern const mp_obj_type_t machine_keypad_type;
#endif



// void machine_timer_deinit_all(void);

#endif /* __MOD_MACHINE_H_ */


