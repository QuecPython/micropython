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

/**
 ******************************************************************************
 * @file    modmachine.c
 * @author  xxx
 * @version V1.0.0
 * @date    2019/04/16
 * @brief   xxx
 ******************************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include "mpconfigport.h"

#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/mperrno.h"

#include "modmachine.h"
#include "shared/runtime/pyexec.h"
#include "mphalport.h"

#if MICROPY_QPY_MODULE_MACHINE
#if MICROPY_PY_SOFT_RESET
static mp_obj_t machine_soft_reset(void) {
    pyexec_system_exit = PYEXEC_FORCED_EXIT;
    MP_STATE_VM(mp_softreset_exception).traceback_data = NULL;
    MP_STATE_MAIN_THREAD(mp_pending_exception) = MP_OBJ_FROM_PTR(&MP_STATE_VM(mp_softreset_exception));
#if MICROPY_ENABLE_SCHEDULER
    if (MP_STATE_VM(sched_state) == MP_SCHED_IDLE) {
        MP_STATE_VM(sched_state) = MP_SCHED_PENDING;
    }
#endif

    mp_mthread_wakeup();//add by freddy @20211227 解决主线程处于交互时,子线程触发软重启无法重启虚拟机的问题

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(machine_soft_reset_obj, machine_soft_reset);
#endif


static const mp_rom_map_elem_t machine_module_globals_table[] = {
	{ MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_machine) },
#if MICROPY_QPY_MACHINE_PIN
	{ MP_ROM_QSTR(MP_QSTR_Pin), MP_ROM_PTR(&machine_pin_type) },
#endif
#if MICROPY_QPY_MACHINE_EXTINT
	{ MP_ROM_QSTR(MP_QSTR_ExtInt), MP_ROM_PTR(&machine_extint_type) },
#endif
#if MICROPY_QPY_MACHINE_UART
	{ MP_ROM_QSTR(MP_QSTR_UART), MP_ROM_PTR(&machine_uart_type) },
#endif
#if MICROPY_QPY_MACHINE_SPI
	{ MP_ROM_QSTR(MP_QSTR_SPI), MP_ROM_PTR(&machine_hard_spi_type) },
#endif
#if MICROPY_QPY_MACHINE_I2C
	{ MP_ROM_QSTR(MP_QSTR_I2C), MP_ROM_PTR(&machine_hard_i2c_type) },
#endif
#if MICROPY_QPY_MACHINE_I2C_SOFT
	{ MP_ROM_QSTR(MP_QSTR_I2C_simulation), MP_ROM_PTR(&machine_simulation_i2c_type) },
#endif
#if MICROPY_QPY_MACHINE_TIMER
    { MP_OBJ_NEW_QSTR(MP_QSTR_Timer), MP_ROM_PTR(&machine_timer_type) },
#endif
#if MICROPY_PY_SOFT_RESET
	{ MP_ROM_QSTR(MP_QSTR_SoftReset), MP_ROM_PTR(&machine_soft_reset_obj) },
#endif
#if MICROPY_QPY_MACHINE_RTC
	{ MP_ROM_QSTR(MP_QSTR_RTC), MP_ROM_PTR(&machine_rtc_type) },
#endif
#if MICROPY_QPY_MACHINE_LCD
	{ MP_ROM_QSTR(MP_QSTR_LCD), MP_ROM_PTR(&machine_lcd_type) },
#endif
#if MICROPY_QPY_MACHINE_WDT
	{ MP_ROM_QSTR(MP_QSTR_WDT), MP_ROM_PTR(&machine_wdt_type) },
#endif
#if MICROPY_QPY_MACHINE_KEYPAD
	{ MP_ROM_QSTR(MP_QSTR_KeyPad), MP_ROM_PTR(&machine_keypad_type) },
#endif
#if MICROPY_QPY_MACHINE_NANDFLASH
	{ MP_ROM_QSTR(MP_QSTR_NANDFLASH), MP_ROM_PTR(&machine_nandflash_type) },
#endif
};

static MP_DEFINE_CONST_DICT(machine_module_globals, machine_module_globals_table);

const mp_obj_module_t mp_module_machine = {
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t *)&machine_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_machine, mp_module_machine);

#endif /* MICROPY_QPY_MODULE_MACHINE */

