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

#ifndef MICROPY_INCLUDED_QUECMOD_VFS_QUECFS_H
#define MICROPY_INCLUDED_QUECMOD_VFS_QUECFS_H

#include "py/obj.h"

typedef struct _mp_obj_vfs_quecfs_t {
    mp_obj_base_t base;
	vstr_t cur_dir;
    bool readonly;
} mp_obj_vfs_quecfs_t;

extern const mp_obj_type_t mp_type_vfs_quecfs;
extern const mp_obj_type_t mp_type_vfs_quecfs_fileio;
extern const mp_obj_type_t mp_type_vfs_quecfs_textio;

mp_obj_t mp_vfs_quecfs_file_open(mp_obj_t self_in, const mp_obj_type_t *type, mp_obj_t file_in, mp_obj_t mode_in);
#endif