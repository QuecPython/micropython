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

#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include "py/obj.h"
#include "py/runtime.h"
#include "mphalport.h"

#if MICROPY_QPY_MISC_ADC

#include "helios_adc.h"

typedef enum
{
	ADC0 = 0,
	ADC1 = 1,
	ADC2 = 2,
	ADC3 = 3,
}ADCn;

typedef struct _misc_adc_obj_t {
    mp_obj_base_t base;
    unsigned int pin;
} misc_adc_obj_t;

const mp_obj_type_t misc_adc_type;


/*=============================================================================*/
/* FUNCTION: Helios__adc_init                                                      */
/*=============================================================================*/
/*!@brief 		: ADC driver init
 * @param[in] 	: 
 * @param[out] 	: 
 * @return		:
 *        -  0--success
 *        - -1--error
 */
/*=============================================================================*/
static mp_obj_t Helios_adc_init(mp_obj_t self_in)
{
	int ret = Helios_ADC_Init();
	return mp_obj_new_int(ret);
}
static MP_DEFINE_CONST_FUN_OBJ_1(Helios_adc_init_obj, Helios_adc_init);


/*=============================================================================*/
/* FUNCTION: Helios_get_cur_source_vol                                            */
/*=============================================================================*/
/*!@brief 		: Read ADC channel voltage
 * @param[in] 	: adc_channel - 0 or 1
 * @param[out] 	: 
 * @return		:
 *        -  If successful,return voltage(mV)
 *        - -1--error
 */
/*=============================================================================*/
static mp_obj_t Helios_adc_read(mp_obj_t self_in, mp_obj_t adc_channel)
{
	int ret = -1;
	int channel = mp_obj_get_int(adc_channel);
	if ((channel == 0) || (channel == 1) || (channel == 2) || (channel == 3))
	{
		unsigned int chl = channel;
		//ret = ql_adc_read(chl, &batvol);
		
		ret = Helios_ADC_Read((Helios_ADCNum) chl);
		return mp_obj_new_int(ret);
	}
	return mp_obj_new_int(-1);
}
static MP_DEFINE_CONST_FUN_OBJ_2(Helios_adc_read_obj, Helios_adc_read);


/*=============================================================================*/
/* FUNCTION: Helios_get_cur_source_vol                                            */
/*=============================================================================*/
/*!@brief 		: read adc1 Voltage
 * @param[in] 	: 
 * @param[out] 	: 
 * @return		: Voltage
 *    
 */
/*=============================================================================*/
/*static mp_obj_t Helios_get_cur_source_vol(mp_obj_t self_in)
{
	int vol = ql_get_cur_source_vol();
	return mp_obj_new_int(vol);
}
static MP_DEFINE_CONST_FUN_OBJ_1(Helios_get_cur_source_vol_obj, qpy_get_cur_source_vol);
*/


/*=============================================================================*/
/* FUNCTION: Helios_adc_deinit                                                    */
/*=============================================================================*/
/*!@brief 		: adc deinit
 * @param[in] 	: 
 * @param[out] 	: 
 * @return		: 0
 *    
 */
/*=============================================================================*/
static mp_obj_t Helios_adc_deinit(mp_obj_t self_in)
{
	int ret = Helios_ADC_Deinit();
	return mp_obj_new_int(ret);
}
static MP_DEFINE_CONST_FUN_OBJ_1(Helios_adc_deinit_obj, Helios_adc_deinit);



static const mp_rom_map_elem_t misc_adc_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&Helios_adc_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&Helios_adc_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&Helios_adc_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_ADC0), MP_ROM_INT(ADC0) },
#if !defined(PLAT_Qualcomm)
    { MP_ROM_QSTR(MP_QSTR_ADC1), MP_ROM_INT(ADC1) },
#if !defined(PLAT_Unisoc_8850) && !defined(BOARD_EC600ECN_LC) && !defined(PLAT_SONY_ALT1350)
    { MP_ROM_QSTR(MP_QSTR_ADC2), MP_ROM_INT(ADC2) },
    { MP_ROM_QSTR(MP_QSTR_ADC3), MP_ROM_INT(ADC3) },
#endif
#endif
};
static MP_DEFINE_CONST_DICT(misc_adc_locals_dict, misc_adc_locals_dict_table);

static mp_obj_t misc_adc_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) 
{
    mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);
    //unsigned int pin_id = mp_obj_get_int(args[0]);

    // create ADC object
    misc_adc_obj_t *self = m_new_obj(misc_adc_obj_t);
    self->base.type = &misc_adc_type;
    //self->pin = pin_id;

    return MP_OBJ_FROM_PTR(self);
}

MP_DEFINE_CONST_OBJ_TYPE(
    misc_adc_type,
    MP_QSTR_ADC,
    MP_TYPE_FLAG_NONE,
    make_new, misc_adc_make_new,
    locals_dict, &misc_adc_locals_dict
    );
#endif /* MICROPY_QPY_MISC_ADC */


