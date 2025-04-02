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
 * @file    modmisc.h
 * @author  Pawn
 * @version V1.0.0
 * @date    2020/08/18
 * @brief   misc
 ******************************************************************************
 */

#ifndef __MOD_MISC_H_
#define __MOD_MISC_H_

#include "py/obj.h"


extern const mp_obj_type_t misc_power_type;
extern const mp_obj_type_t misc_pwm_type;
extern const mp_obj_type_t misc_pwm_v2_type;
extern const mp_obj_type_t misc_adc_type;
extern const mp_obj_type_t misc_usb_type;
#if MICROPY_QPY_MISC_USBNET
extern const mp_obj_module_t misc_usbnet_module;
#endif
extern const mp_obj_type_t misc_powerkey_type;

#if defined(PLAT_RDA)
extern const mp_obj_module_t machine_temperature_type;
#endif

#endif /* __MOD_MISC_H_ */

