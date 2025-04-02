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
#include "string.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "py/runtime.h"
#include "py/nlr.h"
#include "py/objlist.h"
#include "py/objstr.h"

#if MICROPY_QPY_MODULE_ATCMD

#include "helios_debug.h"
#include "helios_atcmd.h"


#define QPY_MODATCMD_LOG(msg, ...)      custom_log("ATCMD", msg, ##__VA_ARGS__)
extern int _get_current_simid(void);

static mp_obj_t qpy_atcmd_send_sync(size_t n_args, const mp_obj_t *args)
{
    int cur_simid = _get_current_simid();
	char *at_cmd = (char *)mp_obj_str_get_str(args[0]);
    char *at_include_str = (char *)mp_obj_str_get_str(args[2]);
    int at_time_out = mp_obj_get_int(args[3]);

	if ((at_cmd == NULL) || (cur_simid != 0) || (at_time_out <= 0))
	{
        QPY_MODATCMD_LOG("invalid param.");
        return mp_obj_new_int(-1);
	}
    HELIOS_AT_RESP_STATUS_E ret = 0;
    mp_buffer_info_t datainfo;
    mp_get_buffer_raise(args[1], &datainfo, MP_BUFFER_READ);
    if (datainfo.len > 0) {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[1], &bufinfo, MP_BUFFER_WRITE);
        MP_THREAD_GIL_EXIT();
        ret = Helios_Atcmd_Send_Sync(cur_simid, at_cmd,bufinfo.buf,bufinfo.len,at_include_str,at_time_out);
        MP_THREAD_GIL_ENTER();
    } else {
        MP_THREAD_GIL_EXIT();
        ret = Helios_Atcmd_Send_Sync(cur_simid, at_cmd,NULL,0,at_include_str,at_time_out);
        MP_THREAD_GIL_ENTER();
    }
    
    return mp_obj_new_int(ret);

}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(qpy_atcmd_send_sync_obj, 4, 4, qpy_atcmd_send_sync);


static const mp_rom_map_elem_t mp_module_atcmd_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),           MP_ROM_QSTR(MP_QSTR_atcmd) },
    { MP_ROM_QSTR(MP_QSTR_sendSync), MP_ROM_PTR(&qpy_atcmd_send_sync_obj) },
};
static MP_DEFINE_CONST_DICT(mp_module_atcmd_globals, mp_module_atcmd_globals_table);


const mp_obj_module_t mp_module_atcmd = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_atcmd_globals,
};


MP_REGISTER_MODULE(MP_QSTR_atcmd, mp_module_atcmd);

#endif



