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
#include "mpconfigport.h"

#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/mperrno.h"

#if MICROPY_QPY_MODULE_UTILS

#include "modutils.h"

static const mp_rom_map_elem_t utils_module_globals_table[] = {
	{ MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_utils) },
	{ MP_ROM_QSTR(MP_QSTR_crc32), MP_ROM_PTR(&mp_crc32_type) },
};

static MP_DEFINE_CONST_DICT(utils_module_globals, utils_module_globals_table);

const mp_obj_module_t mp_module_utils = {
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t *)&utils_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_utils, mp_module_utils);

#endif
