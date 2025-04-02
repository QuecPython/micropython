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

#include "py/runtime.h"
#include "py/gc.h"
#include "py/mphal.h"
#include "py/mperrno.h"

#if MICROPY_QPY_MACHINE_I2C

#include "machine_i2c.h"
#include "helios_iic.h"
#include "mphalport.h"


const mp_obj_type_t machine_hard_i2c_type;

typedef struct _machine_hard_i2c_obj_t {
    mp_obj_base_t base;
	uint32_t bus_id;
	uint32_t fastmode;
} machine_hard_i2c_obj_t;

static void machine_hard_i2c_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_hard_i2c_obj_t *self = self_in;
    mp_printf(print, "I2C%d", self->bus_id);
}

mp_obj_t machine_hard_i2c_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_id, ARG_fastmode };
    static const mp_arg_t allowed_args[] = {
		{ MP_QSTR_id, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_fastmode, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 1} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

	uint32_t bus_id = args[ARG_id].u_int;
	uint32_t fastmode = args[ARG_fastmode].u_int;
    

    if (bus_id > HELIOS_I2C3) {
       mp_raise_ValueError(MP_ERROR_TEXT("bus id must be (0~3)"));
    }
   
    volatile machine_hard_i2c_obj_t *self = m_new_obj(machine_hard_i2c_obj_t);

    self->base.type = &machine_hard_i2c_type;
	self->bus_id = bus_id;
	self->fastmode = fastmode;
	if(0 != Helios_I2C_Init((Helios_I2CEnum) self->bus_id, (Helios_I2CMode) self->fastmode)) {
		return mp_const_false;
	}
    return MP_OBJ_FROM_PTR(self);
}
static const mp_arg_t machine_i2c_mem_allowed_args[] = {
    { MP_QSTR_slaveaddr,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    { MP_QSTR_regaddr, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    { MP_QSTR_regaddr_len, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    { MP_QSTR_databuf,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    { MP_QSTR_datasize, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 8} },
};
static mp_obj_t machine_i2c_write_mem(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_slaveaddr, ARG_regaddr, ARG_regaddr_len, ARG_databuf, ARG_datasize };
    mp_arg_val_t args[MP_ARRAY_SIZE(machine_i2c_mem_allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
        MP_ARRAY_SIZE(machine_i2c_mem_allowed_args), machine_i2c_mem_allowed_args, args);
		
	machine_hard_i2c_obj_t *self = (machine_hard_i2c_obj_t *)MP_OBJ_TO_PTR(pos_args[0]);

	mp_buffer_info_t regaddr_bufinfo;
    mp_get_buffer_raise(args[ARG_regaddr].u_obj, &regaddr_bufinfo, MP_BUFFER_READ);
	int regaddr_length = args[ARG_regaddr_len].u_int;

    // get the buffer to write the data from
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_databuf].u_obj, &bufinfo, MP_BUFFER_READ);
	int length = (size_t)args[ARG_datasize].u_int > bufinfo.len ? bufinfo.len : (size_t)args[ARG_datasize].u_int;
	//uart_printf("i2c wirte: busid:%x slaveddr:%x, regaddr:%x, data:%s datalen:%d\n", self->bus_id, args[ARG_slaveaddr].u_int, args[ARG_regaddr].u_int, 
	//							bufinfo.buf, args[ARG_datasize].u_int);
    // do the transfer
	int ret = HELIOS_I2C_Write((Helios_I2CEnum) self->bus_id, (uint8_t) args[ARG_slaveaddr].u_int, (uint8_t*) regaddr_bufinfo.buf,(size_t) regaddr_length,(void*) bufinfo.buf, (size_t) length);
    if (ret < 0) {
        ret = -1;
        return mp_obj_new_int(ret);
    }

    return mp_obj_new_int(ret);
}
static MP_DEFINE_CONST_FUN_OBJ_KW(machine_i2c_write_obj, 1, machine_i2c_write_mem);

static const mp_arg_t machine_i2c_mem_allowed_read_args[] = {
    { MP_QSTR_slaveaddr,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    { MP_QSTR_regaddr, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    { MP_QSTR_regaddr_len, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    { MP_QSTR_databuf,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    { MP_QSTR_datasize, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 8} },
    { MP_QSTR_dalay, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 8} },
};
static mp_obj_t machine_i2c_read_mem(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_slaveaddr, ARG_regaddr, ARG_regaddr_len, ARG_databuf, ARG_datasize, ARG_dalay };
    mp_arg_val_t args[MP_ARRAY_SIZE(machine_i2c_mem_allowed_read_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
        MP_ARRAY_SIZE(machine_i2c_mem_allowed_read_args), machine_i2c_mem_allowed_read_args, args);
		
	machine_hard_i2c_obj_t *self = (machine_hard_i2c_obj_t *)MP_OBJ_TO_PTR(pos_args[0]);

	mp_buffer_info_t regaddr_bufinfo;
    mp_get_buffer_raise(args[ARG_regaddr].u_obj, &regaddr_bufinfo, MP_BUFFER_READ);
    int regaddr_length = (size_t)args[ARG_regaddr_len].u_int > regaddr_bufinfo.len ? regaddr_bufinfo.len : (size_t)args[ARG_regaddr_len].u_int;
    
    // get the buffer to read the data into
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_databuf].u_obj, &bufinfo, MP_BUFFER_WRITE);
	
	int length = (size_t)args[ARG_datasize].u_int > bufinfo.len ? bufinfo.len : (size_t)args[ARG_datasize].u_int;
	
	//uart_printf("i2c read: busid:%x slaveddr:%x, regaddr:%x, datalen:%d\n", self->bus_id, args[ARG_slaveaddr].u_int, args[ARG_regaddr].u_int, 
	//							args[ARG_datasize].u_int);
    // do the transfer	
	int ret = HELIOS_I2C_Read((Helios_I2CEnum) self->bus_id, (uint8_t) args[ARG_slaveaddr].u_int,(uint8_t*) regaddr_bufinfo.buf, (size_t) regaddr_length, (void*) bufinfo.buf, (size_t) length,(uint32_t) args[ARG_dalay].u_int);
	if (ret < 0) {
        ret = -1;
        return mp_obj_new_int(ret);
    }

    return mp_obj_new_int(ret);
}
static MP_DEFINE_CONST_FUN_OBJ_KW(machine_i2c_read_obj, 1, machine_i2c_read_mem);

static const mp_rom_map_elem_t machine_i2c_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&machine_i2c_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&machine_i2c_write_obj) },
	// class constants
    { MP_ROM_QSTR(MP_QSTR_I2C0),       MP_ROM_INT(HELIOS_I2C0) },  
#if !defined(PLAT_ASR_1803s) && !defined(BOARD_EC800MCN_LA) && !defined(BOARD_EC800MCN_GA) && !defined(BOARD_EC800NCN_LA) && !defined(BOARD_EC800NCN_LA_VOLTE)
    { MP_ROM_QSTR(MP_QSTR_I2C1),       MP_ROM_INT(HELIOS_I2C1) },
#endif        
#if !defined(PLAT_RDA) && !defined(BOARD_EC800NCN_LA) && !defined(BOARD_EC800NCN_LA_VOLTE) && !defined(BOARD_EC600GCN_LD)
    { MP_ROM_QSTR(MP_QSTR_I2C2),       MP_ROM_INT(HELIOS_I2C2) },
#if !defined(PLAT_Qualcomm) && !defined(BOARD_EC800MCN_LA) && !defined(BOARD_EC800MCN_GA) && !defined(BOARD_EC800GCN_GA) && !defined(BOARD_EC800GCN_LD) && !defined(BOARD_EC800GCN_LD_HRXM)
    { MP_ROM_QSTR(MP_QSTR_I2C3),       MP_ROM_INT(HELIOS_I2C3) },
#endif
#endif
    { MP_ROM_QSTR(MP_QSTR_STANDARD_MODE),       MP_ROM_INT(HELIOS_STANDARD_MODE) },
    { MP_ROM_QSTR(MP_QSTR_FAST_MODE),       MP_ROM_INT(HELIOS_FAST_MODE) },
};

MP_DEFINE_CONST_DICT(mp_machine_hard_i2c_locals_dict, machine_i2c_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    machine_hard_i2c_type,
    MP_QSTR_I2C,
    MP_TYPE_FLAG_NONE,
    make_new, machine_hard_i2c_make_new,
    print, machine_hard_i2c_print,
    locals_dict, &mp_machine_hard_i2c_locals_dict
    );

#endif
