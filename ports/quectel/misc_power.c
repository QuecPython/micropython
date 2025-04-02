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

#include <stdio.h>
#include <stdint.h>
#include "stdlib.h"
#include "mpconfigport.h"
#include "py/obj.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/mperrno.h"

#if MICROPY_QPY_MISC_POWER

#include "modmisc.h"
#include "helios_power.h"

const mp_obj_type_t misc_power_type;

static mp_obj_t misc_power_reset()
{
	int ret = 1;
	Helios_Power_Reset(ret);
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(misc_power_reset_obj, misc_power_reset);


static mp_obj_t misc_power_down()
{
	int ret = 1;
	Helios_Power_Down(ret);
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(misc_power_down_obj, misc_power_down);

static mp_obj_t misc_power_get_down_reason()
{
	int ret;
	ret = Helios_Power_GetDownReason();
	return mp_obj_new_int(ret);
}

#if !defined(PLAT_RDA)
static MP_DEFINE_CONST_FUN_OBJ_0(misc_power_get_down_reason_obj, misc_power_get_down_reason);
#endif
static mp_obj_t misc_power_get_up_reason()
{
	int ret = 0;
	ret = Helios_Power_GetUpReason();
	return mp_obj_new_int(ret);
}
static MP_DEFINE_CONST_FUN_OBJ_0(misc_power_get_up_reason_obj, misc_power_get_up_reason);


static mp_obj_t misc_power_get_batt()
{
	unsigned int ret;
	ret = Helios_Power_GetBatteryVol();
	return mp_obj_new_int(ret);
}
static MP_DEFINE_CONST_FUN_OBJ_0(misc_power_get_batt_obj, misc_power_get_batt);

static const mp_rom_map_elem_t misc_power_locals_dict_table[] = {
	{ MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_misc) },
	{ MP_ROM_QSTR(MP_QSTR_powerRestart), MP_ROM_PTR(&misc_power_reset_obj) },
	{ MP_ROM_QSTR(MP_QSTR_powerDown), MP_ROM_PTR(&misc_power_down_obj) },
	{ MP_ROM_QSTR(MP_QSTR_powerOnReason), MP_ROM_PTR(&misc_power_get_up_reason_obj) },
#if !defined(PLAT_RDA)
	{ MP_ROM_QSTR(MP_QSTR_powerDownReason), MP_ROM_PTR(&misc_power_get_down_reason_obj) },
#endif
	{ MP_ROM_QSTR(MP_QSTR_getVbatt), MP_ROM_PTR(&misc_power_get_batt_obj) },
};

static MP_DEFINE_CONST_DICT(misc_power_locals_dict, misc_power_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    misc_power_type,
    MP_QSTR_Power,
    MP_TYPE_FLAG_NONE,
    locals_dict, &misc_power_locals_dict
);
#endif /* MICROPY_QPY_MISC_POWER */
