/**************************************************************************
 @file	modsecuredata.c
 @brief	Packaging the C API for sms into a Python API.

 DESCRIPTION
 This module provide the SIM API of micropython.

 INITIALIZATION AND SEQUENCING REQUIREMENTS


 ===========================================================================
 Copyright (c) 2018 Quectel Wireless Solution, Co., Ltd.All Rights Reserved.
 Quectel Wireless Solution Proprietary and Confidential.
 ===========================================================================

						EDIT HISTORY FOR FILE
This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN		WHO			WHAT,WHERE,WHY
----------  ---------   ---------------------------------------------------
2021/07/07  Mia.zhong	Create.						
**************************************************************************/

#include <stdlib.h>
#include "py/obj.h"
#include "py/runtime.h"
#include "mphalport.h"

#if MICROPY_QPY_MODULE_SECUREDATA

#include "helios_debug.h"
#include "helios_securedata.h"


enum {ARG_index, ARG_databuf, ARG_datalen };

STATIC const mp_arg_t securedata_allowed_args[] = {
	{ MP_QSTR_index,     	MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = -1} },
    { MP_QSTR_databuf,     	MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    { MP_QSTR_datalen,     	MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
};

STATIC mp_obj_t qpy_securedata_read(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    
    mp_arg_val_t args[MP_ARRAY_SIZE(securedata_allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args,
        MP_ARRAY_SIZE(securedata_allowed_args), securedata_allowed_args, args);

	int32_t index = args[ARG_index].u_int;
	if (index < 1 || index > HELIOS_MAX_SECUREDATA_COUNT) {
        return mp_obj_new_int(-1);
        //mp_raise_msg_varg(&mp_type_ValueError, "Index must be greater than 0 and less than %d", HELIOS_MAX_SECUREDATA_COUNT+1);
    }

    // get the buffer to write the data from
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_databuf].u_obj, &bufinfo, MP_BUFFER_READ);
	int length = (int32_t)(args[ARG_datalen].u_int) > (int32_t)bufinfo.len ? (int32_t)bufinfo.len : (int32_t)(args[ARG_datalen].u_int);

    Helios_SecureData_t sec_data = {0};
    sec_data.index = index;
    sec_data.len = length;
    sec_data.pBuffer = (uint8_t *)malloc(sizeof(uint8_t)*length);
    if (sec_data.pBuffer == NULL)
    {
        m_malloc_fail(length);
    }
    
    int ret = Helios_Securedata_Read(&sec_data);
    if (ret < 0) {
        if (sec_data.pBuffer)
        {
            free(sec_data.pBuffer);
            sec_data.pBuffer = NULL;
        }
        return mp_obj_new_int(ret);
    }

    //strncpy((char *)bufinfo.buf, (char *)sec_data.pBuffer,strlen((char *)sec_data.pBuffer));
    memcpy((char *)bufinfo.buf, (char *)sec_data.pBuffer, ret);
    if (sec_data.pBuffer)
    {
        free(sec_data.pBuffer);
        sec_data.pBuffer = NULL;
    }
    return mp_obj_new_int(ret);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(qpy_securedata_read_obj, 2, qpy_securedata_read);


STATIC mp_obj_t qpy_securedata_store(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    mp_arg_val_t args[MP_ARRAY_SIZE(securedata_allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args,
        MP_ARRAY_SIZE(securedata_allowed_args), securedata_allowed_args, args);

	int32_t index = args[ARG_index].u_int;
	if (index < 1 || index > HELIOS_MAX_SECUREDATA_COUNT) {
        return mp_obj_new_int(-1);
        //mp_raise_msg_varg(&mp_type_ValueError, "Index must be greater than 0 and less than %d", HELIOS_MAX_SECUREDATA_COUNT+1);
    }

    // get the buffer to write the data from
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_databuf].u_obj, &bufinfo, MP_BUFFER_READ);
	int length = (int32_t)(args[ARG_datalen].u_int) > (int32_t)bufinfo.len ? (int32_t)bufinfo.len : (int32_t)(args[ARG_datalen].u_int);

    Helios_SecureData_t sec_data = {0};
    sec_data.index = index;
    sec_data.len = length;
    sec_data.pBuffer = (uint8_t *)malloc(sizeof(uint8_t)*length + 1);
    if (sec_data.pBuffer == NULL)
    {
        m_malloc_fail(length);
    }
    memcpy((char *)sec_data.pBuffer, (char *)bufinfo.buf, length);
    sec_data.pBuffer[length] = 0;
    
    int ret = Helios_Securedata_Store(&sec_data);
    if (sec_data.pBuffer)
    {
        free(sec_data.pBuffer);
        sec_data.pBuffer = NULL;
    }
    if (ret < 0) {
        return mp_obj_new_int(ret);
    }

    return mp_obj_new_int(ret);

}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(qpy_securedata_store_obj, 2, qpy_securedata_store);

STATIC const mp_rom_map_elem_t mp_module_securedat_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_Read),         MP_ROM_PTR(&qpy_securedata_read_obj) },
	{ MP_ROM_QSTR(MP_QSTR_Store),        MP_ROM_PTR(&qpy_securedata_store_obj) },
};
STATIC MP_DEFINE_CONST_DICT(mp_module_secureData_global_dict, mp_module_securedat_locals_dict_table);

const mp_obj_module_t module_SecureData = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_secureData_global_dict,
};

#endif /* MICROPY_QPY_MODULE_SECUREDATA */
