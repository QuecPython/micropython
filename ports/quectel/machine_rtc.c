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
#include <string.h>
#include <stdlib.h>

#include "py/nlr.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/mphal.h"

#if MICROPY_QPY_MACHINE_RTC

#include "modmachine.h"
#include "helios_debug.h"
#include "helios_rtc.h"
#include "callbackdeal.h"

#define HELIOS_RTC_LOG(fmt, ...) custom_log(machine_rtc, fmt, ##__VA_ARGS__)


typedef struct _machine_rtc_obj_t {
    mp_obj_base_t base;
} machine_rtc_obj_t;

static c_callback_t *callback_cur = NULL;


// singleton RTC object
//static const machine_rtc_obj_t machine_rtc_obj = {{&machine_rtc_type}};
const mp_obj_type_t machine_rtc_type;
static machine_rtc_obj_t *machine_rtc_obj_ptr = NULL;



static mp_obj_t machine_rtc_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
	if (machine_rtc_obj_ptr != NULL)
	{
		// singleton RTC object
		return MP_OBJ_FROM_PTR(machine_rtc_obj_ptr);
	}
	machine_rtc_obj_ptr = mp_obj_malloc_with_finaliser(machine_rtc_obj_t, &machine_rtc_type);
    machine_rtc_obj_ptr->base.type = &machine_rtc_type;
    return MP_OBJ_FROM_PTR(machine_rtc_obj_ptr);
}



static mp_obj_t machine_rtc_datetime_helper(mp_uint_t n_args, const mp_obj_t *args) {
	//ql_rtc_time_t tm = {0}; //Jayceon-2020/08/27:Resolved that calling the init() function would cause system dump
    Helios_RTCTime tm = {0};
	if (n_args == 1) {
        // Get time

        //struct timeval tv;

       // gettimeofday(&tv, NULL);
        //timeutils_struct_time_t tm;

        //timeutils_seconds_since_2000_to_struct_time(tv.tv_sec, &tm);
		
		//ql_rtc_get_time(&tm);
		// Pawn 2021/4/28 Solve the problem of different initial time zones between RDA and ASR
		// Pawn Fix the bug SW1PQUECPYTHON-119
		Helios_RTC_GetLocalTime(&tm);
		// Pawn Fix the bug SW1PQUECPYTHON-119 end
        mp_obj_t tuple[8] = {
            mp_obj_new_int(tm.tm_year),
            mp_obj_new_int(tm.tm_mon),
            mp_obj_new_int(tm.tm_mday),
            mp_obj_new_int(tm.tm_wday),
            mp_obj_new_int(tm.tm_hour),
            mp_obj_new_int(tm.tm_min),
            mp_obj_new_int(tm.tm_sec),
            mp_obj_new_int(0)
        };

        return mp_obj_new_tuple(8, tuple);
    } else {
        // Set time

        mp_obj_t *items;
        mp_obj_get_array_fixed_n(args[1], 8, &items);
		tm.tm_year = mp_obj_get_int(items[0]);
		tm.tm_mon = mp_obj_get_int(items[1]);
		tm.tm_mday = mp_obj_get_int(items[2]);
		tm.tm_hour = mp_obj_get_int(items[4]);
		tm.tm_min = mp_obj_get_int(items[5]);
		tm.tm_sec = mp_obj_get_int(items[6]);

		//int ret = ql_rtc_set_time(&tm);
		
		// Pawn Fix the bug SW1PQUECPYTHON-119
		int ret = Helios_RTC_NtpSetTime(&tm, 1);
		// Pawn Fix the bug SW1PQUECPYTHON-119 end
		
        //struct timeval tv = {0};
        //tv.tv_sec = timeutils_seconds_since_2000(mp_obj_get_int(items[0]), mp_obj_get_int(items[1]), mp_obj_get_int(items[2]), mp_obj_get_int(items[4]), mp_obj_get_int(items[5]), mp_obj_get_int(items[6]));
        //tv.tv_usec = mp_obj_get_int(items[7]);
        //settimeofday(&tv, NULL);

        return mp_obj_new_int(ret);
    }
}


static mp_obj_t machine_rtc_datetime(mp_uint_t n_args, const mp_obj_t *args) {
    return machine_rtc_datetime_helper(n_args, args);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_rtc_datetime_obj, 1, 2, (mp_obj_t)machine_rtc_datetime);

#if !defined(PLAT_Qualcomm)
static mp_obj_t  machine_rtc_set_alarm(mp_obj_t self_in, mp_obj_t time)
{
	Helios_RTCTime tm = {0};
	mp_obj_t *items;
	
    mp_obj_get_array_fixed_n(time, 8, &items);
	tm.tm_year = mp_obj_get_int(items[0]);
	tm.tm_mon = mp_obj_get_int(items[1]);
	tm.tm_mday = mp_obj_get_int(items[2]);
	tm.tm_hour = mp_obj_get_int(items[4]);
	tm.tm_min = mp_obj_get_int(items[5]);
	tm.tm_sec = mp_obj_get_int(items[6]);
	
	int ret = Helios_RTC_Set_Alarm(&tm);
	
	return mp_obj_new_int(ret);
}
static MP_DEFINE_CONST_FUN_OBJ_2(machine_rtc_set_alarm_obj, machine_rtc_set_alarm);


static mp_obj_t  machine_rtc_enable_alarm(mp_obj_t self_in, mp_obj_t enable)
{
	int on_off = mp_obj_get_int(enable);
	
	int ret = Helios_RTC_Enable_Alarm((unsigned char)on_off);
	
	return mp_obj_new_int(ret);
}
static MP_DEFINE_CONST_FUN_OBJ_2(machine_rtc_enable_alarm_obj, machine_rtc_enable_alarm);


void rtc_callback_to_python(void)
{
	if(callback_cur == NULL) {
		return;
	}
// #if MICROPY_ENABLE_CALLBACK_DEAL
// 	st_CallBack_RTC *rtc_msg = malloc(sizeof(st_CallBack_RTC));
// 	if(NULL != rtc_msg) 
//     {
//     	rtc_msg->callback = *callback_cur;
//     	qpy_send_msg_to_callback_deal_thread(CALLBACK_TYPE_ID_RTC, rtc_msg);
//     }
// #else
//     mp_sched_schedule_ex(callback_cur, mp_const_none);
// #endif

	mp_sched_schedule_ex(callback_cur, mp_const_none);

#if defined(PLAT_RDA) || defined(PLAT_EIGEN) || defined(PLAT_EIGEN_718)
	Helios_Rtc_Set_Called(Helios_Rtc_Call_NoCalled);
#endif
}


static mp_obj_t  machine_rtc_register_callback(mp_obj_t self_in, mp_obj_t callback)
{
    static c_callback_t cb = {0};
    memset(&cb, 0, sizeof(c_callback_t));
	callback_cur = &cb;
	mp_sched_schedule_callback_register(callback_cur, callback);

#if defined(PLAT_RDA) || defined(PLAT_EIGEN) || defined(PLAT_EIGEN_718)
	if ( Helios_Rtc_Get_Called()  == Helios_Rtc_Call_Called  )
	{
		mp_sched_schedule_ex(callback_cur, NULL);
		Helios_Rtc_Set_Called(Helios_Rtc_Call_NoCalled);
	}
#endif
	int ret = Helios_RTC_Register_cb(rtc_callback_to_python);
	
	return mp_obj_new_int(ret);
}
static MP_DEFINE_CONST_FUN_OBJ_2(machine_rtc_register_callback_obj, machine_rtc_register_callback);
#endif

static mp_obj_t machine_rtc_init(mp_obj_t self_in, mp_obj_t date) {
    mp_obj_t args[2] = {self_in, date};
    machine_rtc_datetime_helper(2, args);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(machine_rtc_init_obj, machine_rtc_init);

static mp_obj_t machine_rtc_deinit(mp_obj_t self_in) {
    HELIOS_RTC_LOG("machine rtc deinit.\r\n");
    callback_cur = NULL;
	machine_rtc_obj_ptr = NULL;
    return mp_obj_new_int(0);
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_rtc_deinit_obj, machine_rtc_deinit);


static const mp_rom_map_elem_t machine_rtc_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_rtc_init_obj) },
	{ MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&machine_rtc_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_datetime), MP_ROM_PTR(&machine_rtc_datetime_obj) },
#if !defined(PLAT_Qualcomm)
    { MP_ROM_QSTR(MP_QSTR_set_alarm), MP_ROM_PTR(&machine_rtc_set_alarm_obj) },
    { MP_ROM_QSTR(MP_QSTR_enable_alarm), MP_ROM_PTR(&machine_rtc_enable_alarm_obj) },
    { MP_ROM_QSTR(MP_QSTR_register_callback), MP_ROM_PTR(&machine_rtc_register_callback_obj) },
#endif
};
static MP_DEFINE_CONST_DICT(machine_rtc_locals_dict, machine_rtc_locals_dict_table);

// const mp_obj_type_t machine_rtc_type = {
//     { &mp_type_type },
//     .name = MP_QSTR_RTC,
//     .make_new = machine_rtc_make_new,
//     .locals_dict = (mp_obj_t)&machine_rtc_locals_dict,
// };

MP_DEFINE_CONST_OBJ_TYPE(
    machine_rtc_type,
    MP_QSTR_RTC,
    MP_TYPE_FLAG_NONE,
    make_new, machine_rtc_make_new,
    locals_dict, &machine_rtc_locals_dict
    );

#endif /* MICROPY_QPY_MACHINE_RTC */

