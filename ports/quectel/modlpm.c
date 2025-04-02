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
 * @file    modlpm.c
 * @author  Pawn.zhou
 * @version V1.0.0
 * @date    2020/10/24
 * @brief   Low power dependent API
 ******************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "py/obj.h"
#include "py/runtime.h"
#include "mpconfigport.h"

#if MICROPY_QPY_MODULE_LPM

#include "helios_lpm.h"
#if defined(PLAT_RDA)
#include "helios_debug.h"
#endif

/*=============================================================================*/
/* FUNCTION: Helios_sleep_enable_mp                                             */
/*=============================================================================*/
/*!@brief     : Set the system sleep switch
 * @param[in]   : s_flag, 0-false, 1-true
 * @param[out]  : 
 * @return    :
 *        -  0--successful
 */
/*=============================================================================*/
static mp_obj_t Helios_sleep_enable_mp(const mp_obj_t s_flag)
{
  int ret = 0;
  int sleep_flag;
  
  sleep_flag = mp_obj_get_int(s_flag);
  if(sleep_flag == 0 || sleep_flag ==1){
	ret = Helios_LPM_AutoSleepEnable((uint32_t) sleep_flag);
    return mp_obj_new_int(ret);
  }else{
	return mp_obj_new_int(-1);
  }

}
static MP_DEFINE_CONST_FUN_OBJ_1(Helios_sleep_enable_mp_obj, Helios_sleep_enable_mp);


/*=============================================================================*/
/* FUNCTION: Helios_lpm_wakelock_lock_mp                                             */
/*=============================================================================*/
/*!@brief     : lock
 * @param[in]   : 
 * @param[out]  : 
 * @return    :
 *        -  0--successful
 *        -  -1--error
 */
/*=============================================================================*/
static mp_obj_t Helios_lpm_wakelock_lock_mp(const mp_obj_t lpm_fd)
{
  int ret = 0;
  int g_lpm_fd;
  
  g_lpm_fd = mp_obj_get_int(lpm_fd);
  
  ret = Helios_LPM_WakeLockAcquire(g_lpm_fd);
  if ( ret == 0 )
  {
    return mp_obj_new_int(0);
  }
  
  return mp_obj_new_int(-1);
}
#if !defined(PLAT_RDA)

static MP_DEFINE_CONST_FUN_OBJ_1(Helios_lpm_wakelock_lock_mp_obj, Helios_lpm_wakelock_lock_mp);
#endif


/*=============================================================================*/
/* FUNCTION: Helios_lpm_wakelock_unlock_mp                                             */
/*=============================================================================*/
/*!@brief     : unlock
 * @param[in]   : 
 * @param[out]  : 
 * @return    :
 *        -  0--successful
  *       -  -1--error
 */
/*=============================================================================*/
static mp_obj_t Helios_lpm_wakelock_unlock_mp(const mp_obj_t lpm_fd)
{
  int ret = 0;
  int g_lpm_fd;
  
  g_lpm_fd = mp_obj_get_int(lpm_fd);
  ret = Helios_LPM_WakeLockRelease(g_lpm_fd);
  if ( ret == 0 )
  {
    return mp_obj_new_int(0);
  }
  
  return mp_obj_new_int(-1);
}
#if !defined(PLAT_RDA)

static MP_DEFINE_CONST_FUN_OBJ_1(Helios_lpm_wakelock_unlock_mp_obj, Helios_lpm_wakelock_unlock_mp);
#endif

/*=============================================================================*/
/* FUNCTION: Helios_lpm_wakelock_create_mp                                             */
/*=============================================================================*/
/*!@brief     : create wakelock
 * @param[in]   : lock_name
 * @param[in]   : name_size
 * @param[out]  : 
 * @return    :
 *        -  wakelock_fd
 */
/*=============================================================================*/
static mp_obj_t Helios_lpm_wakelock_create_mp(size_t n_args, const mp_obj_t *args)
{
  int wakelock_fd;
  int name_size;
  
  char *lock_name = (char *)mp_obj_str_get_str(args[0]);
  name_size = strlen(lock_name);
  
#if 0   //Aim At  EC100Y by kingka
  ret = quec_lpm_task_init();
  if (ret != 0)
  {
    return -1;
  }
#endif

  if((NULL == lock_name) || (0 == name_size))
  {
	return mp_obj_new_int(-1);
  }
  	
  wakelock_fd = Helios_LPM_WakeLockInit(lock_name, name_size);

  
  /*if ( wakelock_fd < 1 || wakelock_fd > 32 )
  {
    return mp_obj_new_int(-1);
  }*/
  // Helios_autosleep_enable(1);
  
  return mp_obj_new_int(wakelock_fd);
}
#if !defined(PLAT_RDA)

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(Helios_lpm_wakelock_create_mp_obj, 1, 2, Helios_lpm_wakelock_create_mp);
#endif

/*=============================================================================*/
/* FUNCTION: Helios_lpm_wakelock_delete_mp                                             */
/*=============================================================================*/
/*!@brief     : delete wakelock
 * @param[in]   : wakelock_fd
 * @param[out]  : 
 * @return    :
 *        -  0
 */
/*=============================================================================*/
static mp_obj_t Helios_lpm_wakelock_delete_mp(const mp_obj_t lpm_fd)
{
  int ret = 0;
  int g_lpm_fd;
  
  g_lpm_fd = mp_obj_get_int(lpm_fd);
  ret = Helios_LPM_WakeLockDeInit(g_lpm_fd);
  return mp_obj_new_int(ret);
}
#if !defined(PLAT_RDA)
static MP_DEFINE_CONST_FUN_OBJ_1(Helios_lpm_wakelock_delete_mp_obj, Helios_lpm_wakelock_delete_mp);
#endif

/*=============================================================================*/
/* FUNCTION: Helios_lpm_get_wakelock_num_mp                                             */
/*=============================================================================*/
/*!@brief     : get wakelock num
 * @param[in]   : 
 * @param[out]  : 
 * @return    :
 *        -  (int)wakelock num
 */
/*=============================================================================*/

static mp_obj_t Helios_lpm_get_wakelock_num_mp()
{
  int num;
  
  num = Helios_LPM_GetWakeLockNum();
  
  return mp_obj_new_int(num);
}
#if !defined(PLAT_RDA)
static MP_DEFINE_CONST_FUN_OBJ_0(Helios_lpm_get_wakelock_num_mp_obj, Helios_lpm_get_wakelock_num_mp);
#endif

/*=============================================================================*/
/* FUNCTION: Helios_lpm_ex_set_mp                                             */
/*=============================================================================*/
/*!@brief     : set power ex
 * @param[in]   : lpm seconds
 * @param[out]  :
 * @return    :
 *        0     success
 *		  else  fail
 */
/*=============================================================================*/
#if defined(PLAT_ASR) || defined(PLAT_Unisoc)
static mp_obj_t Helios_lpm_ex_set_mp(size_t n_args, const mp_obj_t *args)
{
	int ret = 0;
	int mode = mp_obj_get_int(args[0]);
	int no_data_time = mp_obj_get_int(args[1]);
	int punish_time = mp_obj_get_int(args[2]);

	ret = Helios_LPM_EXSet((uint8_t)mode, (uint8_t)no_data_time, (uint16_t)punish_time);
	if ( ret == 0 )
	{
		return mp_obj_new_int(0);
	}

	return mp_obj_new_int(-1);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(Helios_lpm_ex_set_mp_obj, 3, 3, Helios_lpm_ex_set_mp);
#endif

/*
static mp_obj_t mp_get_free_size()
{
  int num;
  
  num = Helios_rtos_get_free_heap_size();
  
  return mp_obj_new_int(num);
}
static MP_DEFINE_CONST_FUN_OBJ_0(mp_get_free_size_obj, mp_get_free_size);
*/
#if defined(PLAT_RDA)
static c_callback_t *helios_sleep_callback = NULL;
uint32_t helios_sleep_callback_to_python(uint8_t *buffer,uint len)
{
	mp_obj_t decode_cb[2] = {
		mp_obj_new_str((char *)buffer, len),
	 	mp_obj_new_int((mp_int_t)len),
    };
	if( helios_sleep_callback == NULL) {
		return -1;
	}
  mp_sched_schedule_ex(helios_sleep_callback, MP_OBJ_FROM_PTR(mp_obj_new_list(2, decode_cb)));
	return 0;
}

static mp_obj_t helios_sleep_set_callback(mp_obj_t callback)
{
	static c_callback_t cb = {0};
    memset(&cb, 0, sizeof(c_callback_t));
	helios_sleep_callback = &cb;
	mp_sched_schedule_callback_register(helios_sleep_callback, callback);
	
	Helios_LPM_Sleep_Register_cb((void * )helios_sleep_callback_to_python);
	return mp_obj_new_int(0);
}
static MP_DEFINE_CONST_FUN_OBJ_1(machine_set_sleep_cb_obj, helios_sleep_set_callback);
#endif
#if defined(PLAT_RDA) || defined(PLAT_Unisoc_8850) || defined(PLAT_Unisoc) || defined(PLAT_EIGEN)
static mp_obj_t helios_pm_set_psm_time(size_t n_args, const mp_obj_t *args)
{
  int tau_uint = 0;
  int tau_time = 0;
  int act_uint = 0;
  int act_time = 0;
  int mode     = 1;
  if ( n_args != 4 && n_args != 1 ){
    return mp_const_false;
  }
  if ( n_args == 4 )
  {
    if ( mp_obj_is_int(args[0]) && mp_obj_is_int(args[1]) \
         && mp_obj_is_int(args[2]) && mp_obj_is_int(args[3])  ){
        tau_uint = mp_obj_get_int(args[0]);
        tau_time = mp_obj_get_int(args[1]);
        act_uint = mp_obj_get_int(args[2]);
        act_time = mp_obj_get_int(args[3]);
    }else{
      return mp_const_false;
    }
  }else{
    mode = mp_obj_get_int(args[0]);
  }
  int ret = Helios_LPM_Set_PsmTime(mode,tau_uint,tau_time,act_uint,act_time);
  if ( ret != 0 ){
    return mp_const_false;
  }
	return mp_const_true;
}

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(helios_pm_set_psm_time_obj, 1, 4, helios_pm_set_psm_time);

static mp_obj_t helios_pm_get_psm_time(void)
{
  Helios_psm_info psm_info;
  memset((void *)&psm_info,0x00,sizeof(psm_info));
  int ret = Helios_LPM_get_PsmTime(&psm_info);
  if ( ret != 0 ){
    return mp_const_none;
  }
  mp_obj_t decode_cb[5] = {
		  mp_obj_new_int((mp_int_t)psm_info.mode),
	 	  mp_obj_new_int((mp_int_t)psm_info.tau_uint),
      mp_obj_new_int((mp_int_t)psm_info.tau_time),
      mp_obj_new_int((mp_int_t)psm_info.act_uint),
      mp_obj_new_int((mp_int_t)psm_info.act_time),
    };
  return mp_obj_new_list(5, decode_cb);
}

static MP_DEFINE_CONST_FUN_OBJ_0(helios_pm_get_psm_time_obj,  helios_pm_get_psm_time);
#if  defined(PLAT_RDA) 
static mp_obj_t helios_pm_set_edrx_time(size_t n_args, const mp_obj_t *args)
{
  Helios_edrx_info edrx_info;
  memset((void *)&edrx_info,0x00,sizeof(edrx_info));

  
  if ( n_args != 3 && n_args != 1 ){
    return mp_const_false;
  }

  if ( n_args == 3 )
  {
    if ( mp_obj_is_int(args[0]) && mp_obj_is_int(args[1]) \
         && mp_obj_is_str(args[2]) ){
        edrx_info.i8_mode = mp_obj_get_int(args[0]);
        edrx_info.i8_ActType = mp_obj_get_int(args[1]);
        size_t str_len = 0;
        const char *strRequested_eDRX = mp_obj_str_get_data(args[2], &str_len);
        if ( str_len > 4 || strRequested_eDRX == NULL ){
          return mp_const_false;
        }


        if ( edrx_info.i8_mode < 0  || edrx_info.i8_mode > 3 ||\
         (edrx_info.i8_ActType != 0 && edrx_info.i8_ActType != 5) ){
          return mp_const_false;
        }
        memcpy(edrx_info.str5_Requested_eDRX,strRequested_eDRX,str_len);
    }else{

      return mp_const_false;
    }
  }else{

    edrx_info.i8_mode     = mp_obj_get_int(args[0]);
    edrx_info.i8_ActType  = -1;
  }
  int ret = Helios_LPM_set_eDrx(edrx_info);
  if ( ret != 0 ){
    return mp_const_false;
  }
	return mp_const_true;
}

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(helios_pm_set_edrx_time_obj, 1, 4, helios_pm_set_edrx_time);

static mp_obj_t helios_pm_get_edrx_time(void)
{
  Helios_edrx_info edrx_info;
  memset((void *)&edrx_info,0x00,sizeof(edrx_info));
  int ret = Helios_LPM_get_eDrx(&edrx_info);
  if ( ret != 0 ){
    return mp_const_none;
  }
  mp_obj_t decode_cb[5] = {
		  mp_obj_new_int((mp_int_t)edrx_info.i8_mode),
	 	  mp_obj_new_int((mp_int_t)edrx_info.i8_ActType),
      mp_obj_new_str((char *)edrx_info.str5_Requested_eDRX,strlen((char *)edrx_info.str5_Requested_eDRX)),
      mp_obj_new_str((char *)edrx_info.str5_Nwprovided_eDRX,strlen((char *)edrx_info.str5_Nwprovided_eDRX)),
      mp_obj_new_str((char *)edrx_info.str5_Paging_time_window,strlen((char *)edrx_info.str5_Paging_time_window)),
  };
  return mp_obj_new_list(5, decode_cb);
}

static MP_DEFINE_CONST_FUN_OBJ_0(helios_pm_get_edrx_time_obj,  helios_pm_get_edrx_time);

#endif
#endif

extern const mp_obj_type_t lpm_wakeup_type;

#if defined(PLAT_EIGEN)
static mp_obj_t helios_simpowersave_enable(size_t n_args,const mp_obj_t *args) 
{   
    int ret = 0;  
    if(n_args == 1)
    {       
        int value = mp_obj_get_int(args[0]);
        if((value == 0) || (value == 1))
        {
    	    ret = Helios_LPM_SET_SimPowerSave(value);
        }
        else
        {
            ret = -1;
        }
    }
    else
    {
        ret = Helios_LPM_GET_SimPowerSave();
    } 
    return mp_obj_new_int(ret);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(helios_simpowersave_enable_obj, 0, 1, helios_simpowersave_enable);

static mp_obj_t helios_force_hib(void)
{
    return mp_obj_new_int(Helios_LPM_force_hib());
}
static MP_DEFINE_CONST_FUN_OBJ_0(helios_force_hib_obj,  helios_force_hib);
#endif

static const mp_rom_map_elem_t pm_module_globals_table[] = {
  { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_pm) },
#if !defined(PLAT_RDA)
  { MP_ROM_QSTR(MP_QSTR_create_wakelock), MP_ROM_PTR(&Helios_lpm_wakelock_create_mp_obj) },
  { MP_ROM_QSTR(MP_QSTR_delete_wakelock), MP_ROM_PTR(&Helios_lpm_wakelock_delete_mp_obj) },
  { MP_ROM_QSTR(MP_QSTR_wakelock_lock), MP_ROM_PTR(&Helios_lpm_wakelock_lock_mp_obj) },
  { MP_ROM_QSTR(MP_QSTR_wakelock_unlock), MP_ROM_PTR(&Helios_lpm_wakelock_unlock_mp_obj) },
  { MP_ROM_QSTR(MP_QSTR_get_wakelock_num), MP_ROM_PTR(&Helios_lpm_get_wakelock_num_mp_obj) },
#endif
  { MP_ROM_QSTR(MP_QSTR_autosleep), MP_ROM_PTR(&Helios_sleep_enable_mp_obj) },
  // { MP_ROM_QSTR(MP_QSTR_getFreeSize), MP_ROM_PTR(&mp_get_free_size_obj) },
#if defined(PLAT_ASR) || defined(PLAT_Unisoc)
  { MP_ROM_QSTR(MP_QSTR_sclkEx_set), MP_ROM_PTR(&Helios_lpm_ex_set_mp_obj) },
#endif
#if defined(PLAT_RDA)
  { MP_ROM_QSTR(MP_QSTR_set_sleep_cb), MP_ROM_PTR(&machine_set_sleep_cb_obj) },
#endif
#if defined(PLAT_RDA) || defined(PLAT_Unisoc_8850) || defined(PLAT_Unisoc) || defined(PLAT_EIGEN)
  { MP_ROM_QSTR(MP_QSTR_set_psm_time), MP_ROM_PTR(&helios_pm_set_psm_time_obj) },
  { MP_ROM_QSTR(MP_QSTR_get_psm_time), MP_ROM_PTR(&helios_pm_get_psm_time_obj) },
#if defined(PLAT_RDA)
  { MP_ROM_QSTR(MP_QSTR_set_edrx_time), MP_ROM_PTR(&helios_pm_set_edrx_time_obj) },
  { MP_ROM_QSTR(MP_QSTR_get_edrx_time), MP_ROM_PTR(&helios_pm_get_edrx_time_obj) },
#endif
#endif

#if MICROPY_QPY_LPM_WAKEUP
  { MP_ROM_QSTR(MP_QSTR_WakeUp), MP_ROM_PTR(&lpm_wakeup_type) },  //add for eigen plat 20221116
#endif

#if defined(PLAT_EIGEN)
  { MP_ROM_QSTR(MP_QSTR_Simpowersave), MP_ROM_PTR(&helios_simpowersave_enable_obj) },
  { MP_ROM_QSTR(MP_QSTR_Forcehib), MP_ROM_PTR(&helios_force_hib_obj) },
#endif

};
static MP_DEFINE_CONST_DICT(pm_module_globals, pm_module_globals_table);

const mp_obj_module_t mp_module_pm = {
  .base = { &mp_type_module },
  .globals = (mp_obj_dict_t *)&pm_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_pm, mp_module_pm);
#endif /* MICROPY_QPY_MODULE_PM */
