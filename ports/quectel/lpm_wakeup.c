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
#include <string.h>
#include <stdlib.h>

#include "py/runtime.h"
#include "py/gc.h"
#include "py/mphal.h"

#if MICROPY_QPY_LPM_WAKEUP

#include "helios_wakeup.h"
#include "helios_debug.h"
#include "callbackdeal.h"

typedef struct _machine_uart_obj_t {
    mp_obj_base_t base;
    unsigned char wakeup_id;
    unsigned char wakeup_pull;
} lpm_wakeup_obj_t;

typedef void(*wakeup_handler_t)(uint8_t pin_level);

static lpm_wakeup_obj_t *wakeup_self_obj[HELIOS_WAKEUP_MAX] = {0};

static c_callback_t callback_wakeup[HELIOS_WAKEUP_MAX] = {0};

const mp_obj_type_t lpm_wakeup_type;

#if MICROPY_ENABLE_CALLBACK_DEAL
#define WAKEUP_CALLBACK_OP(WAKEUP_ID, pin_level)    do{                                                                                       \
                                                        st_CallBack_Wakeup *wakeup_msg = malloc(sizeof(st_CallBack_Wakeup));                  \
	                                                    if(NULL != wakeup_msg) {                                                              \
	                                                        wakeup_msg->level = pin_level;                                                    \
    	                                                    wakeup_msg->callback = callback_wakeup[WAKEUP_ID];                                \
    	                                                    qpy_send_msg_to_callback_deal_thread(CALLBACK_TYPE_ID_WAKEUP, wakeup_msg);        \
    	                                                }                                                                                     \
                                                    }while(0)                                                                                 
#else
#define WAKEUP_CALLBACK_OP(WAKEUP_ID, pin_level)   do{                                                                                                   \
                                                        mp_sched_schedule_ex(callback_wakeup[WAKEUP_ID], MP_OBJ_FROM_PTR(mp_obj_new_int(pin_level)));    \
                                                   }while(0)

#endif



#define WAKEUP_CB_PY(X) 								   \
static void wakeuphandler##X(uint8_t pin_level)            \
{	                                                       \
	if(callback_wakeup[X].cb != mp_const_none &&           \
    ((mp_sched_num_pending() < MICROPY_SCHEDULER_DEPTH)))  \
    {                                                      \
		WAKEUP_CALLBACK_OP(X, pin_level);                  \
	}                                                      \
    else{                                                  \
        return;                                            \
    }                                                      \
}

WAKEUP_CB_PY(0)
WAKEUP_CB_PY(1)
WAKEUP_CB_PY(2)
WAKEUP_CB_PY(3)
WAKEUP_CB_PY(4)
WAKEUP_CB_PY(5)

wakeup_handler_t wakeup_handler[HELIOS_WAKEUP_MAX] = {
	wakeuphandler0,
    wakeuphandler1,
    wakeuphandler2,
    wakeuphandler3,
    wakeuphandler4,
    wakeuphandler5
};
/******************************************************************************/
// MicroPython bindings for WAKEUP

STATIC void lpm_wakeup_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    lpm_wakeup_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "WAKEUP%u, PULL=%u",self->wakeup_id, self->wakeup_pull);
}

STATIC void lpm_wakeup_init_helper(lpm_wakeup_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) 
{
	enum {ARG_wakeup_id, ARG_wakeup_pull};
    static const mp_arg_t allowed_args[] = 
	{
        { MP_QSTR_wakeup_id, MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_wakeup_pull, MP_ARG_INT, {.u_int = -1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    switch (args[ARG_wakeup_id].u_int) {
		case HELIOS_WAKEUP_0:
		case HELIOS_WAKEUP_1:
		case HELIOS_WAKEUP_2:
		case HELIOS_WAKEUP_3:
		case HELIOS_WAKEUP_4:
		case HELIOS_WAKEUP_5:		
				
            self->wakeup_id = args[ARG_wakeup_id].u_int;
            break;
        default:
            mp_raise_ValueError(MP_ERROR_TEXT("invalid wakeup_id"));
            break;
    }
	helios_debug("args[ARG_wakeup_id].u_int=%d,args[ARG_wakeup_pull].u_int=%d",args[ARG_wakeup_id].u_int,args[ARG_wakeup_pull].u_int);
	if((args[ARG_wakeup_pull].u_int == 0) || (args[ARG_wakeup_pull].u_int == 1))
	{
    	self->wakeup_pull = args[ARG_wakeup_pull].u_int;
	}
	else
	{
		mp_raise_ValueError(MP_ERROR_TEXT("invalid wakeup_pullmode"));
	}

	if(Helios_Wakeup_Config((Helios_Wakeup_ID)self->wakeup_id,(unsigned char)self->wakeup_pull)!=0)
	{
		mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Wakeup(%d) init fail"), self->wakeup_id);
	}
}

STATIC mp_obj_t lpm_wakeup_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // get wakeup id
    helios_debug("args[0]=%d,args[1]=%d,n_args=%d",mp_obj_get_int(args[0]),mp_obj_get_int(args[1]),n_args);
    mp_int_t wakeup_id = mp_obj_get_int(args[0]);
    if (wakeup_id < HELIOS_WAKEUP_0 || wakeup_id >= HELIOS_WAKEUP_MAX) 
	{
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("WAKEUP(%d) does not exist"), wakeup_id);
    }
    
    // create instance
    if (wakeup_self_obj[wakeup_id] == NULL)
    {
    	wakeup_self_obj[wakeup_id] = mp_obj_malloc_with_finaliser(lpm_wakeup_obj_t, &lpm_wakeup_type);
    }
	lpm_wakeup_obj_t *self = wakeup_self_obj[wakeup_id];
	
    self->base.type = &lpm_wakeup_type;

    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
    lpm_wakeup_init_helper(self, n_args, args, &kw_args);

    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t lpm_wakeup_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    lpm_wakeup_init_helper(args[0], n_args - 1, args + 1, kw_args);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(lpm_wakeup_init_obj, 1, lpm_wakeup_init);


STATIC mp_obj_t lpm_wakeup_enable(mp_obj_t self_in) {
    lpm_wakeup_obj_t *self = MP_OBJ_TO_PTR(self_in);
	int ret = Helios_Wakeup_Enable((Helios_Wakeup_ID) self->wakeup_id);
	helios_debug("Helios_Wakeup_Enable =%d",self->wakeup_id);
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(lpm_wakeup_enable_obj, lpm_wakeup_enable);


STATIC mp_obj_t lpm_wakeup_disable(mp_obj_t self_in) {
    lpm_wakeup_obj_t *self = MP_OBJ_TO_PTR(self_in);
	int ret = Helios_Wakeup_Disable((Helios_Wakeup_ID) self->wakeup_id);
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(lpm_wakeup_disable_obj, lpm_wakeup_disable);


STATIC mp_obj_t lpm_wakeup_get_value(mp_obj_t self_in) {
    lpm_wakeup_obj_t *self = MP_OBJ_TO_PTR(self_in);
	int ret = Helios_Wakeup_Get_Pin_Value((Helios_Wakeup_ID) self->wakeup_id);
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(lpm_wakeup_get_value_obj, lpm_wakeup_get_value);


STATIC mp_obj_t lpm_wakeup_deinit(mp_obj_t self_in) {
    lpm_wakeup_obj_t *self = MP_OBJ_TO_PTR(self_in);
	int ret = Helios_Wakeup_Deinit((Helios_Wakeup_ID) self->wakeup_id);
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(lpm_wakeup_deinit_obj, lpm_wakeup_deinit);

STATIC mp_obj_t lpm_getwakestate(mp_obj_t self_in) {
    lpm_wakeup_obj_t *self = MP_OBJ_TO_PTR(self_in);
	int ret = Helios_Wakeup_ReadWakePinvalue((Helios_Wakeup_ID) self->wakeup_id);
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(lpm_getwakestate_obj, lpm_getwakestate);

STATIC mp_obj_t lpm_getwaketriggered(mp_obj_t self_in) {
    lpm_wakeup_obj_t *self = MP_OBJ_TO_PTR(self_in);
	int ret = Helios_Wakeup_WakeTrip((Helios_Wakeup_ID) self->wakeup_id);
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(lpm_getwaketriggered_obj, lpm_getwaketriggered);

STATIC mp_obj_t lpm_set_callback(mp_obj_t self_in, mp_obj_t callback) {
    lpm_wakeup_obj_t *self = MP_OBJ_TO_PTR(self_in);

	mp_sched_schedule_callback_register(&(callback_wakeup[self->wakeup_id]), callback);

	int ret = Helios_Wakeup_set_callback((Helios_Wakeup_ID) self->wakeup_id, (Helios_WakeupPad_Callback)wakeup_handler[self->wakeup_id]);
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(lpm_set_callback_obj, lpm_set_callback);

STATIC const mp_rom_map_elem_t lpm_wakeup_locals_dict_table[] = {
	{ MP_ROM_QSTR(MP_QSTR___del__), 	MP_ROM_PTR(&lpm_wakeup_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&lpm_wakeup_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_enable), MP_ROM_PTR(&lpm_wakeup_enable_obj) },
    { MP_ROM_QSTR(MP_QSTR_disable), MP_ROM_PTR(&lpm_wakeup_disable_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&lpm_wakeup_get_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&lpm_wakeup_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_wakestat), MP_ROM_PTR(&lpm_getwakestate_obj) },
    { MP_ROM_QSTR(MP_QSTR_waketriggered), MP_ROM_PTR(&lpm_getwaketriggered_obj) },    
    { MP_ROM_QSTR(MP_QSTR_set_callback), MP_ROM_PTR(&lpm_set_callback_obj) },
	
#if !defined(BOARD_EC800ECN_LC) && !defined(BOARD_EC800ECN_LQ)
	{ MP_ROM_QSTR(MP_QSTR_WAKEUP0), MP_ROM_INT(HELIOS_WAKEUP_0) },
	{ MP_ROM_QSTR(MP_QSTR_WAKEUP3), MP_ROM_INT(HELIOS_WAKEUP_3) },
	{ MP_ROM_QSTR(MP_QSTR_WAKEUP4), MP_ROM_INT(HELIOS_WAKEUP_4) },
#endif
	{ MP_ROM_QSTR(MP_QSTR_WAKEUP1), MP_ROM_INT(HELIOS_WAKEUP_1) },
	{ MP_ROM_QSTR(MP_QSTR_WAKEUP2), MP_ROM_INT(HELIOS_WAKEUP_2) },
	{ MP_ROM_QSTR(MP_QSTR_WAKEUP5), MP_ROM_INT(HELIOS_WAKEUP_5) },
};

STATIC MP_DEFINE_CONST_DICT(lpm_wakeup_locals_dict, lpm_wakeup_locals_dict_table);


const mp_obj_type_t lpm_wakeup_type = {
    { &mp_type_type },
    .name = MP_QSTR_WakeUp,
    .print = lpm_wakeup_print,
    .make_new = lpm_wakeup_make_new,
    .locals_dict = (mp_obj_dict_t *)&lpm_wakeup_locals_dict,
};

#endif


