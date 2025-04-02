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

#include "py/objstr.h"
#include "py/runtime.h"
#include "py/mperrno.h"
#include "py/mphal.h"

#include "quectel_version.h"

static mp_obj_t helios_platform(void) {	
    char platform[64] = {0};
    snprintf(platform, sizeof(platform), "%s%d.%d.%d", "heliossdk-v", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
    return mp_obj_new_str(platform, strlen(platform));
}
static MP_DEFINE_CONST_FUN_OBJ_0(helios_platform_obj, helios_platform);

static const mp_rom_map_elem_t helios_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_helios) },

    { MP_ROM_QSTR(MP_QSTR_platform), MP_ROM_PTR(&helios_platform_obj) },
};

static MP_DEFINE_CONST_DICT(helios_module_globals, helios_module_globals_table);

const mp_obj_module_t helios_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&helios_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_helios, helios_module);