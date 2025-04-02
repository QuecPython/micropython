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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "py/builtin.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "py/objmodule.h"
#include "py/stackctrl.h"
#include "py/mphal.h"
#include "shared/runtime/pyexec.h"
#include "shared/readline/readline.h"
#include "shared/runtime/gchelper.h"
#include <stdlib.h>
#include "helios.h"
#include "helios_os.h"
#include "helios_debug.h"
#include "mphalport.h"
#include "callbackdeal.h"
#if MICROPY_KBD_EXCEPTION
#include "shared/runtime/interrupt_char.h"
#endif

#if CONFIG_MBEDTLS
#include "mbedtls_init.h"
#endif


Helios_Thread_t ql_micropython_task_ref;


#define MP_TASK_STACK_SIZE      (MP_QPY_TASK_STACK_SIZE)
#define MP_TASK_STACK_LEN       (MP_TASK_STACK_SIZE/sizeof(uint32_t))

#if (defined(PLAT_ASR) || defined(PLAT_Unisoc) || defined(PLAT_ASR_1803s) || defined(PLAT_ASR_1803sc) || defined(PLAT_Qualcomm) \
    || defined(PLAT_ASR_1606) || defined(PLAT_ASR_1609) || defined(PLAT_Unisoc_8910_R05) || defined(PLAT_Unisoc_8910_R06) || defined(PLAT_ASR_1602)) //support
#define QPY_ASSERT_SUPPORT 1
#endif

void nlr_jump_fail(void *val) {
#if QPY_ASSERT_SUPPORT
    Helios_Assert(__func__, __FILE__, __LINE__, "");
#endif
    while(1);
}

void NORETURN __fatal_error(const char *msg) {
#if QPY_ASSERT_SUPPORT
    Helios_Assert(msg, __FILE__, __LINE__, "");
#endif
    while(1);
}

#ifndef NDEBUG
#if !defined(PLAT_Qualcomm)
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
#if QPY_ASSERT_SUPPORT
    Helios_Assert(expr, file, line, "");
#else
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
#endif
    __fatal_error("Assertion failed");
}

#else
void __assert_fail(const char *__message,
                                            const char *__file, int __line,
                                            const char *__function) {
#if QPY_ASSERT_SUPPORT
   Helios_Assert(__message, __file, __line, "");
#else
   printf("Assertion '%s' failed, at file %s:%d\n", __message, __file, __line);
#endif
   __fatal_error("Assertion failed");
}
#endif

#endif


/*
int _lseek() {return 0;}
int _read() {return 0;}
int _write() {return 0;}
int _close() {return 0;}
void _exit(int x) {for(;;){}}
int _sbrk() {return 0;}
int _kill() {return 0;}
int _getpid() {return 0;}
int _fstat() {return 0;}
int _isatty() {return 0;}
*/
/*
void *malloc(size_t n) {return NULL;}
void *calloc(size_t nmemb, size_t size) {return NULL;}
void *realloc(void *ptr, size_t size) {return NULL;}
void free(void *p) {}
int printf(const char *m, ...) {return 0;}
void *memcpy(void *dest, const void *src, size_t n) {return NULL;}
int memcmp(const void *s1, const void *s2, size_t n) {return 0;}
void *memmove(void *dest, const void *src, size_t n) {return NULL;}
void *memset(void *s, int c, size_t n) {return NULL;}
int strcmp(const char *s1, const char* s2) {return 0;}
int strncmp(const char *s1, const char* s2, size_t n) {return 0;}
size_t strlen(const char *s) {return 0;}
char *strcat(char *dest, const char *src) {return NULL;}
char *strchr(const char *dest, int c) {return NULL;}
#include <stdarg.h>
int vprintf(const char *format, va_list ap) {return 0;}
int vsnprintf(char *str,  size_t  size,  const  char  *format, va_list ap) {return 0;}

#undef putchar
int putchar(int c) {return 0;}
int puts(const char *s) {return 0;}

void _start(void) {main(0, NULL);}
*/
static char *stack_top;
#if MICROPY_ENABLE_GC
#if defined(PLAT_ECR6600)
static char __attribute__((__section__(".data"))) heap[MICROPY_GC_HEAP_SIZE];
#elif defined(PLAT_SONY_ALT1350)
static char __attribute__((__section__("gpm1_working_data"))) heap[MICROPY_GC_HEAP_SIZE];
#else
static char heap[MICROPY_GC_HEAP_SIZE];
#endif
#endif

extern pyexec_mode_kind_t pyexec_mode_kind;
// extern void machine_timer_deinit_all(void);

#if MICROPY_PY_KBD_EXCEPTION
MAINPY_RUNNING_FLAG_DEF
MAINPY_INTERRUPT_BY_KBD_FLAG_DEF
SET_MAINPY_RUNNING_TIMER_DEF
#endif

#if MICROPY_PY_SOFT_RESET
static int vm_softreset_flag = 0;
#define SOFTRESET_FLAG_SET() vm_softreset_flag = 1;
#define SOFTRESET_FLAG_CLEAR() vm_softreset_flag = 0;
#define SOFTRESET_FLAG_TRUE() (1 == vm_softreset_flag)
#define SOFTRESET_FLAG_FALSE() (0 == vm_softreset_flag)
#endif
void quecpython_task(void *arg)
{
	int stack_dummy;
    Helios_Thread_t id = 0;
	void *stack_ptr = NULL;
    #if MICROPY_QPY_MODULE_POC && CONFIG_POC_BND_XIN
    helios_debug( "start qpy_poc_register_task"); 
    extern void qpy_poc_register_task();
    qpy_poc_register_task();
    #endif
    //Added by Freddy @20210520 在线程sleep时，通过wait queue的方式超时代替实际的sleep,
    //当有callback执行时，给此queue发消息即可快速执行callback
    mp_mthread_sleep_deal_init();

//#if !defined(PLAT_RDA)
#if CONFIG_MBEDTLS
    mbedtls_platform_setup(NULL);
#endif
//#endif

	#if MICROPY_PY_THREAD
    id = Helios_Thread_GetID();
    ql_micropython_task_ref = id;
    stack_ptr = Helios_Thread_GetStaskPtr(id);
#if defined(PLAT_ECR6600) || defined(PLAT_aic8800m40)
    mp_thread_init(stack_ptr, MP_TASK_STACK_SIZE);   // unit: Word
#else
    mp_thread_init(stack_ptr, MP_TASK_STACK_LEN);
#endif
	#endif

	if(mp_hal_stdio_init()) return;

soft_reset:
	mp_stack_set_top((void *)&stack_dummy);
#if defined(PLAT_ECR6600) || defined(PLAT_aic8800m40)  // unit: Byte
    mp_stack_set_limit(MP_TASK_STACK_SIZE * sizeof(uint32_t) - 1024);
#else
    mp_stack_set_limit(MP_TASK_STACK_SIZE - 1024);
#endif
    stack_top = (char*)&stack_dummy;
    #if MICROPY_ENABLE_GC
    gc_init(heap, heap + sizeof(heap));
    #endif
    mp_init();
    #if MICROPY_ENABLE_CALLBACK_DEAL
	qpy_callback_deal_init();
    #endif
    readline_init0();
    
	// run boot-up scripts
#if defined(PLAT_RDA)
    pyexec_frozen_module("_boot_RDA.py");
#elif defined(PLAT_Qualcomm)
	pyexec_frozen_module("_boot_Qualcomm.py");
#elif defined(BOARD_EC800ECN_LC_WDF)
    pyexec_frozen_module("_boot_WDF.py");//EIGEN WDF CUNSTOMER BOOT WITH SINGEL FILE SYSTEM
#elif defined(PLAT_ECR6600) || defined(PLAT_aic8800m40)
    pyexec_frozen_module("_boot_WIFI.py");
#elif defined(BOARD_EC600GCN_LA_CDD)
    pyexec_frozen_module("_boot_dsds.py");
#elif defined(PLAT_SONY_ALT1350)
    pyexec_frozen_module("_boot_SONY.py", false);
#else
    pyexec_frozen_module("_boot.py", false);
#endif

    if (pyexec_mode_kind == PYEXEC_MODE_FRIENDLY_REPL
    #if MICROPY_PY_KBD_EXCEPTION
    && MAINPY_INTERRUPT_BY_KBD_FLAG_FALSE()
    #endif
    ) 
    {
        #if MICROPY_PY_SOFT_RESET
        SOFTRESET_FLAG_CLEAR();
        #endif
        
        #if MICROPY_PY_KBD_EXCEPTION
        MAINPY_RUNNING_FLAG_SET();
        #endif
        
        int ret = pyexec_file_if_exists("/usr/main.py");
        if (ret & PYEXEC_FORCED_EXIT) {
            goto soft_reset_exit;
        }
        
        #if MICROPY_PY_KBD_EXCEPTION
        MAINPY_RUNNING_FLAG_CLEAR();
        if(RET_KBD_INTERRUPT == ret)
        {
            MAINPY_INTERRUPT_BY_KBD_FLAG_SET();
        }
        #endif
        
        #if MICROPY_PY_SOFT_RESET
        if((PYEXEC_SOFTRESET & ret) == PYEXEC_SOFTRESET) {
            SOFTRESET_FLAG_SET();
        }
        #endif
    }
    else
    {
        #if MICROPY_PY_KBD_EXCEPTION
        MAINPY_INTERRUPT_BY_KBD_FLAG_CLEAR();
        #endif
        #if MICROPY_PY_SOFT_RESET
        SOFTRESET_FLAG_CLEAR();
        #endif
    }

    if(1
        #if MICROPY_PY_KBD_EXCEPTION
        && MAINPY_INTERRUPT_BY_KBD_FLAG_FALSE()
        #endif
        #if MICROPY_PY_SOFT_RESET
        && SOFTRESET_FLAG_FALSE()
        #endif
    )
    {
        #if MICROPY_PY_SOFT_RESET
        nlr_buf_t nlr;
        nlr.ret_val = NULL;
        if (nlr_push(&nlr) == 0) {
        #endif
        	for (;;) {
                if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
                    if (pyexec_raw_repl() != 0) {
                        break;
                    }
                } else {
                    if (pyexec_friendly_repl() != 0) {
                        break;
                    }
                }
            }
        #if MICROPY_PY_SOFT_RESET
            nlr_pop();
        } else {
            mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(nlr.ret_val));
        }
        #endif
    }

soft_reset_exit:
#if MICROPY_QPY_MACHINE_TIMER
    // machine_timer_deinit_all();
#endif

	#if MICROPY_PY_THREAD
	//uart_printf("mp_thread_deinit in quecpython task.\r\n");
    mp_thread_deinit();
    #endif

    //mp_module_deinit_all();

	gc_sweep_all();

    mp_hal_stdout_tx_str("MPY: soft reboot\r\n");
    #if MICROPY_ENABLE_CALLBACK_DEAL
    qpy_callback_deal_deinit();
    qpy_callback_para_link_free_all();
    #endif
	mp_deinit();
#if !defined(PLAT_RDA)
    fflush(stdout);
#endif
    goto soft_reset;
}
/* void gc_collect(void) {
    // WARNING: This gc_collect implementation doesn't try to get root
    // pointers from CPU registers, and thus may function incorrectly.
    void *dummy;
    gc_collect_start();
    gc_collect_root(&dummy, ((mp_uint_t)stack_top - (mp_uint_t)&dummy) / sizeof(mp_uint_t));
    gc_collect_end();
    gc_dump_info();
}
 */

#if !MICROPY_VFS
mp_lexer_t *mp_lexer_new_from_file(qstr filename) {
    mp_raise_OSError(MP_ENOENT);
}

mp_import_stat_t mp_import_stat(const char *path) {
    (void)path;
    return MP_IMPORT_STAT_NO_EXIST;
}

mp_obj_t mp_builtin_open(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);

#endif

#if defined(PLAT_aic8800m40)
static Helios_Thread_t taskRef = 0;
void ql_app_main()
{
    Helios_ThreadAttr ThreadAttr = {
            .name = "quecpython_task",
            .stack_size = MP_TASK_STACK_SIZE,
            .priority = 100,
            .entry = quecpython_task,
            .argv = NULL
        };
    taskRef = Helios_Thread_Create(&ThreadAttr);
}
#elif defined(PLAT_SONY_ALT1350)
static Helios_Thread_t qpy_task_ref = 0;
void quecpython_startup(void)
{
    

    Helios_ThreadAttr ThreadAttr = {
            .name = "quecpython_task",
            .stack_size = MP_TASK_STACK_SIZE,
            .priority = 100,
            .entry = quecpython_task,
            .argv = NULL
        };
    qpy_task_ref = Helios_Thread_Create(&ThreadAttr);
}
#else
application_init(quecpython_task, "quecpython_task", (MP_TASK_STACK_SIZE)/1024, 0);

#endif

