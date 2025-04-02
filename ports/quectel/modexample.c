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
#include <stdlib.h>
#include <string.h>
#include "py/compile.h"
#include "py/runtime0.h"
#include "py/nlr.h"
#include "py/objlist.h"
#include "py/objstr.h"
#include "py/runtime.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/stream.h"
#include "shared/runtime/pyexec.h"
#include "shared/runtime/interrupt_char.h"


#if MICROPY_QPY_MODULE_EXAMPLE

//void do_str(const char *src, mp_parse_input_kind_t input_kind) {
//    nlr_buf_t nlr;
//    if (nlr_push(&nlr) == 0) {
//        mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src, strlen(src), 0);
//        qstr source_name = lex->source_name;
//        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);
//        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, true);
//        mp_call_function_0(module_fun);
//        nlr_pop();
//    } else {
//        // uncaught exception
//        mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
//    }
//}


//static mp_obj_t example_exec(const mp_obj_t arg0)
//{
//	QFILE * fp = NULL;
//	int ret, fsize = 0;
//	char fname[128] = {0};
//	mp_buffer_info_t bufinfo;
//	
//	mp_get_buffer_raise(arg0, &bufinfo, MP_BUFFER_READ);
//	uart_printf("example_exec: %s\r\n", bufinfo.buf);
//	if(strlen(bufinfo.buf) > 0)
//	{
//		snprintf(fname, sizeof(fname), "%s/%s", "U:", bufinfo.buf);
//		fp = ql_fopen(fname, "r");
//		if(fp == NULL)
//		{
//			mp_raise_ValueError("file open failed.");
//		}
//		else
//		{
//			fsize = ql_fsize(fp);
//			char *content = malloc(fsize + 1);
//			if(content == NULL)
//			{
//				ql_fclose(fp);
//				mp_raise_ValueError("malloc error.");
//				return mp_const_none;
//			}
//			else
//			{
//				ret = ql_fread((void*)content, fsize, 1, fp);
//				if(ret != fsize)
//				{
//					ql_fclose(fp);
//					mp_raise_ValueError("read error.");
//				}
//				else
//				{
//					content[fsize] = '\0';
//					do_str(content, MP_PARSE_FILE_INPUT);
//				}
//				free(content);
//			}
//		}
//	}
//	else
//	{
//		mp_raise_ValueError("file not exit.");
//	}
//	return mp_const_none;
//}

static mp_obj_t example_exec(const mp_obj_t arg0)
{
	int ret = 0;
	
	mp_buffer_info_t bufinfo;
	char fname[128] = {0};
	char path[128] = {0};
	mp_get_buffer_raise(arg0, &bufinfo, MP_BUFFER_READ);

	memcpy(path, bufinfo.buf, bufinfo.len);
	
	if(bufinfo.buf != NULL)
	{
		// Pawn 2021-01-18 for JIRA STASR3601-2428 begin
		if (path[0] != '/')
		{
			snprintf(fname, sizeof(fname), "/%s", (char *)bufinfo.buf);
		}
		else
		{
			snprintf(fname, sizeof(fname), "%s", (char *)bufinfo.buf);
		}
		
		ret = pyexec_file_if_exists(fname);
	}
	if ( ret == -1 )
	{
		mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("File path error or not exist: [%s]"), (char *)bufinfo.buf);
	}
	#if MICROPY_PY_KBD_EXCEPTION
	else if(ret == RET_KBD_INTERRUPT)
	{
        pyexec_system_exit = PYEXEC_FORCED_EXIT;
        MAINPY_INTERRUPT_BY_KBD_FLAG_SET();
        mp_raise_msg_varg(&mp_type_SystemExit, MP_ERROR_TEXT("CTRL_C Interrupt"));
	}
	#endif
	#if MICROPY_PY_SOFT_RESET
	else if((ret & PYEXEC_SOFTRESET) == PYEXEC_SOFTRESET)
	{
        pyexec_system_exit = PYEXEC_FORCED_EXIT;
        mp_raise_msg_varg(&mp_type_SystemExit, MP_ERROR_TEXT("SoftReset"));
	}
	#endif
	// Pawn 2021-01-18 for JIRA STASR3601-2428 end
	return mp_const_none;
}



static MP_DEFINE_CONST_FUN_OBJ_1(example_exec_obj, example_exec);


static mp_obj_t example_initialize()
{
    static int initialized = 0;
    if (!initialized) {
        initialized = 1;
    }
    return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_0(example_initialize_obj, example_initialize);

static const mp_rom_map_elem_t mp_module_example_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_example) },
    { MP_ROM_QSTR(MP_QSTR___init__), MP_ROM_PTR(&example_initialize_obj) },
    { MP_ROM_QSTR(MP_QSTR_exec), MP_ROM_PTR(&example_exec_obj) },

};


static MP_DEFINE_CONST_DICT(mp_module_example_globals, mp_module_example_globals_table);


const mp_obj_module_t example_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_example_globals,
};

MP_REGISTER_MODULE(MP_QSTR_example, example_module);
#endif /* MICROPY_QPY_MODULE_EXAMPLE */
