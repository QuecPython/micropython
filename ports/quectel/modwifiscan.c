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
#include <stdlib.h>

#include "py/nlr.h"
#include "py/objlist.h"
#include "py/objstr.h"
#include "py/runtime.h"
#include "py/mperrno.h"

#if MICROPY_QPY_MODULE_WIFISCAN
#include "callbackdeal.h"
#include "helios_debug.h"
#include "helios_wifiscan.h"


#define MOD_WIFISCAN_LOG(msg, ...)      custom_log(wifiscan, msg, ##__VA_ARGS__)

static c_callback_t *g_wifiscan_callback = NULL;


/*=============================================================================*/
/* FUNCTION: qpy_wifiscan_support                                              */
/*=============================================================================*/
/*!@brief 		: Determine whether the current platform supports WiFi Scan.
 * @return		:
 *        -  0--not support
 *        -  1--support
 */
/*=============================================================================*/
STATIC mp_obj_t qpy_wifiscan_support(void)
{
	int ret = Helios_WifiScan_Support();
	if (ret == 1)
		return mp_obj_new_bool(1);
	else
		return mp_obj_new_bool(0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_wifiscan_support_obj, qpy_wifiscan_support);

/*=============================================================================*/
/* FUNCTION: qpy_wifiscan_control                                              */
/*=============================================================================*/
/*!@brief 		: Determine whether the current platform supports WiFi Scan.
 * @option		 [in]	 0-disable, 1-enable
 * @return		:
 *        -  0--successful
 *        -  -1--failed
 */
/*=============================================================================*/
STATIC mp_obj_t qpy_wifiscan_control(mp_obj_t option)
{
	int ret = 0;
	int opt = mp_obj_get_int(option);

	if ((opt != 0) && (opt != 1))
	{
		mp_raise_ValueError(MP_ERROR_TEXT("invalid value, option should be 0 or 1."));
	}

	if (opt == 0)
    {
        ret = Helios_WifiScan_Close();
    }
    else if (opt == 1)
    {
        ret = Helios_WifiScan_Open();
    }
    
	return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_wifiscan_control_obj, qpy_wifiscan_control);


/*=============================================================================*/
/* FUNCTION: qpy_wifiscan_get_state                                            */
/*=============================================================================*/
/*!@brief 		: get status of wifiscan.
 * @return		:
 *        -  0--disable
 *        -  1--enable
 */
/*=============================================================================*/
STATIC mp_obj_t qpy_wifiscan_get_state(void)
{
	uint8_t status = 0;
	Helios_WifiScan_GetStatus(&status);
	if (status == 1)
		return mp_obj_new_bool(1);
	else
		return mp_obj_new_bool(0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_wifiscan_get_state_obj, qpy_wifiscan_get_state);


/*=============================================================================*/
/* FUNCTION: qpy_wifiscan_set_configuration                                    */
/*=============================================================================*/
/*!@brief 		: config wifi-scan.
 * @timeout		 	[in]	 The timeout for the upper application
 * @round		 	[in]	 Number of scanning
 * @max_bssid_num	[in]	 The maximum number of hotspots
 * @scan_timeout	[in]	 The timeout for wifi scan
 * @priority		[in]	 WiFi scanning  priority
 * @return		:
 *        -  0--successful
 *        -  -1--failed
 */
/*=============================================================================*/
STATIC mp_obj_t qpy_wifiscan_set_configuration(size_t n_args, const mp_obj_t *args)
{
	int ret = 0;
	Helios_WifiScanConfigStruct info = {0};
	info.timeout = mp_obj_get_int(args[0]);
	info.round = mp_obj_get_int(args[1]);
	info.max_ap_nums = mp_obj_get_int(args[2]);
	info.scan_time = mp_obj_get_int(args[3]);
	info.priority = mp_obj_get_int(args[4]);
	MOD_WIFISCAN_LOG("timeout=%d,round=%d,max_nums=%d,scantime=%d,priority=%d", \
					info.timeout, info.round, info.max_ap_nums, info.scan_time, info.priority);	
#if defined(PLAT_ASR)
	if ((info.timeout < 4) || (info.timeout > 255))
	{
		mp_raise_ValueError(MP_ERROR_TEXT("invalid value, timeout should be in [4,255]."));
	}

	if ((info.round < 1) || (info.round > 3))
	{
		mp_raise_ValueError(MP_ERROR_TEXT("invalid value, round should be in [1,3]."));
	}

	if ((info.max_ap_nums < 4) || (info.max_ap_nums > 30))
	{
		mp_raise_ValueError(MP_ERROR_TEXT("invalid value, max_bssid_num should be in [4,30]."));
	}

	if ((info.scan_time < 1) || (info.scan_time > 255))
	{
		mp_raise_ValueError(MP_ERROR_TEXT("invalid value, scan_time_out should be in [1,255]."));
	}

	if ((info.priority != 0) && (info.priority != 1))
	{
		mp_raise_ValueError(MP_ERROR_TEXT("invalid value, priority should be in [0,1]."));
	}
#elif defined(PLAT_Unisoc) || defined(PLAT_Unisoc_8850)
    if ((info.timeout < 120) || (info.timeout > 5000))
	{
		mp_raise_ValueError(MP_ERROR_TEXT("invalid value, timeout should be in [120,5000]."));
	}

	if ((info.round < 1) || (info.round > 10))
	{
		mp_raise_ValueError(MP_ERROR_TEXT("invalid value, round should be in [1,10]."));
	}

	if ((info.max_ap_nums < 1) || (info.max_ap_nums > 300))
	{
		mp_raise_ValueError(MP_ERROR_TEXT("invalid value, max_bssid_num should be in [1,300]."));
	}

	if ((info.scan_time < 1) || (info.scan_time > 255))
	{
		mp_raise_ValueError(MP_ERROR_TEXT("invalid value, scan_time_out should be in [1,255]."));
	}

	if ((info.priority != 0) && (info.priority != 1))
	{
		mp_raise_ValueError(MP_ERROR_TEXT("invalid value, priority should be in [0,1]."));
	}
#endif
	ret = Helios_WifiScan_SetConfiguration(&info);
	return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(qpy_wifiscan_set_configuration_obj, 5, 5, qpy_wifiscan_set_configuration);


/*=============================================================================*/
/* FUNCTION: qpy_wifiscan_get_configuration                                    */
/*=============================================================================*/
/*!@brief 		: get wifi scan config info.
 * @return		:
 *        -  0--successful
 *        -  -1--failed
 */
/*=============================================================================*/
STATIC mp_obj_t qpy_wifiscan_get_configuration(void)
{
	int ret = 0;
	Helios_WifiScanConfigStruct config_info = {0};
	
	ret = Helios_WifiScan_GetConfiguration(&config_info);
	if(ret == 0)
	{
		mp_obj_t info[5] = 
		{
			mp_obj_new_int(config_info.timeout),
			mp_obj_new_int(config_info.round),
			mp_obj_new_int(config_info.max_ap_nums),
			mp_obj_new_int(config_info.scan_time),
			mp_obj_new_int(config_info.priority),
		};
		return mp_obj_new_tuple(5, info);
	}
	return mp_obj_new_int(-1);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_wifiscan_get_configuration_obj, qpy_wifiscan_get_configuration);


/*=============================================================================*/
/* FUNCTION: qpy_wifiscan_sync_start                                           */
/*=============================================================================*/
/*!@brief 		: Synchronize mode for scanning.
 * @return		:
 *        -  
 *        -  -1--failed
 */
/*=============================================================================*/
STATIC mp_obj_t qpy_wifiscan_sync_start(void)
{
	int i = 0, ret = 0;
	char wifi_addr[30] = {0};
	uint8_t status = 0;
	Helios_WifiScanAPInfoStruct info = {0};

	Helios_WifiScan_GetStatus(&status);
	if (status == 0)
	{
		mp_raise_ValueError(MP_ERROR_TEXT("WiFi-Scan is not enabled."));
	}

	MP_THREAD_GIL_EXIT();
	ret = Helios_WifiScan_SyncStart(&info);
	MP_THREAD_GIL_ENTER();
	if (ret == 0)
	{
		mp_obj_t list_wifi = mp_obj_new_list(0, NULL);
		for (i=0; i<info.ap_nums; i++)
		{
			sprintf(wifi_addr, "%02X:%02X:%02X:%02X:%02X:%02X", \
								info.ap[i].bssid[0],info.ap[i].bssid[1],\
								info.ap[i].bssid[2],info.ap[i].bssid[3],\
								info.ap[i].bssid[4],info.ap[i].bssid[5]);
			mp_obj_t wifi_tuple[2] = 
			{
				mp_obj_new_str(wifi_addr, strlen(wifi_addr)),
				mp_obj_new_int(info.ap[i].rssi)
			};
			mp_obj_list_append(list_wifi, mp_obj_new_tuple(2, wifi_tuple));
		}
		mp_obj_t tuple[2] = 
		{
			mp_obj_new_int(info.ap_nums),
			list_wifi,
		};
		return mp_obj_new_tuple(2, tuple);
	}
	return mp_obj_new_int(-1);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_wifiscan_sync_start_obj, qpy_wifiscan_sync_start);


/*=============================================================================*/
/* FUNCTION: qpy_wifiscan_async_start                                          */
/*=============================================================================*/
/*!@brief 		: Asynchronous mode for scanning.
 * @return		:
 *        -  
 *        -  -1--failed
 */
/*=============================================================================*/
STATIC mp_obj_t qpy_wifiscan_async_start(void)
{
	uint8_t status = 0;
	Helios_WifiScan_GetStatus(&status);
	if (status == 0)
	{
		mp_raise_ValueError(MP_ERROR_TEXT("WiFi-Scan is not enabled."));
	}
	int ret = Helios_WifiScan_AsyncStart();
	return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_wifiscan_async_start_obj, qpy_wifiscan_async_start);


static void qpy_wifiscan_callback(uint8_t msg_id, void *ctx)
{
	uint8_t event_id = msg_id;
	
	if (event_id == HELIOS_WIFISCAN_EVENT_DO_IND)
	{
		if (g_wifiscan_callback)
		{
		#if MICROPY_ENABLE_CALLBACK_DEAL
			st_CallBack_WifiScan *cb_msg = malloc(sizeof(st_CallBack_WifiScan));
		    if(cb_msg != NULL) {

				Helios_WifiScanAPInfoStruct *ap_info = malloc(sizeof(Helios_WifiScanAPInfoStruct));
				if (ap_info == NULL)
				{
					MOD_WIFISCAN_LOG("malloc failed.");
					free(cb_msg);
				}
				memcpy((void *)ap_info, (const void *)ctx, sizeof(Helios_WifiScanAPInfoStruct));
		        cb_msg->msg = ap_info;
	    	    cb_msg->callback = *g_wifiscan_callback;
	    	    qpy_send_msg_to_callback_deal_thread(CALLBACK_TYPE_ID_WIFISCAN, cb_msg);
	    	}
	    #else
			uint16_t i = 0;
			char wifi_addr[30] = {0};
			mp_obj_t list_wifi = mp_obj_new_list(0, NULL);
			MOD_WIFISCAN_LOG("[wifi-scan] callback start.\r\n");
			MP_THREAD_GIL_ENTER();
            Helios_WifiScanAPInfoStruct *ap_info = (Helios_WifiScanAPInfoStruct *)ctx;

    		for (i=0; i<ap_info->ap_nums; i++)
    		{
    			sprintf(wifi_addr, "%02X:%02X:%02X:%02X:%02X:%02X", \
    								ap_info->ap[i].bssid[0],ap_info->ap[i].bssid[1],\
    								ap_info->ap[i].bssid[2],ap_info->ap[i].bssid[3],\
    								ap_info->ap[i].bssid[4],ap_info->ap[i].bssid[5]);

    			mp_obj_t wifi_tuple[2] = 
    			{
    				mp_obj_new_str(wifi_addr, strlen(wifi_addr)),
    				mp_obj_new_int(ap_info->ap[i].rssi)
    			};
    			mp_obj_list_append(list_wifi, mp_obj_new_tuple(2, wifi_tuple));
    		}

    		mp_obj_t tuple[2] = 
    		{
    			mp_obj_new_int(ap_info->ap_nums),
    			list_wifi,
    		};
			mp_sched_schedule_ex(g_wifiscan_callback, mp_obj_new_tuple(2, tuple));
            MP_THREAD_GIL_EXIT();
            
			MOD_WIFISCAN_LOG("[wifi-scan] callback end.\r\n");
		#endif
		}
	}
}

void qpy_wifiscan_msg_proc(void *msg)
{
	uint16_t i = 0;
	char wifi_addr[30] = {0};
	mp_obj_t list_wifi = mp_obj_new_list(0, NULL);
	st_CallBack_WifiScan *cb_msg = (st_CallBack_WifiScan *)msg;
	Helios_WifiScanAPInfoStruct *ap_info = (Helios_WifiScanAPInfoStruct *)cb_msg->msg;

	for (i=0; i<ap_info->ap_nums; i++)
	{
		sprintf(wifi_addr, "%02X:%02X:%02X:%02X:%02X:%02X", \
							ap_info->ap[i].bssid[0],ap_info->ap[i].bssid[1],\
							ap_info->ap[i].bssid[2],ap_info->ap[i].bssid[3],\
							ap_info->ap[i].bssid[4],ap_info->ap[i].bssid[5]);

		mp_obj_t wifi_tuple[2] = 
		{
			mp_obj_new_str(wifi_addr, strlen(wifi_addr)),
			mp_obj_new_int(ap_info->ap[i].rssi)
		};
		mp_obj_list_append(list_wifi, mp_obj_new_tuple(2, wifi_tuple));
	}

	mp_obj_t tuple[2] = 
	{
		mp_obj_new_int(ap_info->ap_nums),
		list_wifi,
	};
	mp_sched_schedule_ex(&cb_msg->callback, mp_obj_new_tuple(2, tuple));
	free(ap_info);
}

/*=============================================================================*/
/* FUNCTION: qpy_wifiscan_register_usr_cb                                      */
/*=============================================================================*/
/*!@brief 		: register the callback function for user
 * @user_cb		[in] 	callback function
 * @return		:
 *        -  0--successful
 *        - -1--error
 */
/*=============================================================================*/
STATIC mp_obj_t qpy_wifiscan_register_usr_cb(mp_obj_t callback)
{	
    static c_callback_t cb = {0};
    memset(&cb, 0, sizeof(c_callback_t));
    g_wifiscan_callback = &cb;
    mp_sched_schedule_callback_register(g_wifiscan_callback, callback);

	Helios_WifiScanInitStruct info = {0};
	info.user_cb = qpy_wifiscan_callback;
	Helios_WifiScan_Init(&info);
	return mp_obj_new_int(0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(qpy_wifiscan_register_usr_cb_obj, qpy_wifiscan_register_usr_cb);

STATIC mp_obj_t qpy_module_wifiscan_deinit(void)
{
	MOD_WIFISCAN_LOG("module wifiScan deinit.\r\n");
	g_wifiscan_callback = NULL;
	Helios_WifiScan_Deinit();
	return mp_obj_new_int(0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(qpy_module_wifiscan_deinit_obj, qpy_module_wifiscan_deinit);

STATIC const mp_rom_map_elem_t mp_module_wifiscan_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), 	MP_ROM_QSTR(MP_QSTR_wifiScan) 				},
    { MP_ROM_QSTR(MP_QSTR___qpy_module_deinit__),   MP_ROM_PTR(&qpy_module_wifiscan_deinit_obj) },
	{ MP_ROM_QSTR(MP_QSTR_support), 	MP_ROM_PTR(&qpy_wifiscan_support_obj) 		},
	{ MP_ROM_QSTR(MP_QSTR_control), 	MP_ROM_PTR(&qpy_wifiscan_control_obj) 		},
	{ MP_ROM_QSTR(MP_QSTR_getState), 	MP_ROM_PTR(&qpy_wifiscan_get_state_obj) 	},
    { MP_ROM_QSTR(MP_QSTR_start), 		MP_ROM_PTR(&qpy_wifiscan_sync_start_obj) 	},
	{ MP_ROM_QSTR(MP_QSTR_asyncStart), 	MP_ROM_PTR(&qpy_wifiscan_async_start_obj) 	},
    { MP_ROM_QSTR(MP_QSTR_getConfig), 	MP_ROM_PTR(&qpy_wifiscan_get_configuration_obj)	},
	{ MP_ROM_QSTR(MP_QSTR_setConfig), 	MP_ROM_PTR(&qpy_wifiscan_set_configuration_obj)	},
	{ MP_ROM_QSTR(MP_QSTR_setCallback), MP_ROM_PTR(&qpy_wifiscan_register_usr_cb_obj) 	},
};
STATIC MP_DEFINE_CONST_DICT(mp_module_wifiscan_globals, mp_module_wifiscan_globals_table);


const mp_obj_module_t mp_module_wifiscan = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_wifiscan_globals,
};

#endif /* MICROPY_QPY_MODULE_WIFISCAN */
