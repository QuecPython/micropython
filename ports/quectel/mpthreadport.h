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

#ifndef MICROPY_INCLUDED_MPTHREADPORT_H
#define MICROPY_INCLUDED_MPTHREADPORT_H

#include "helios_os.h"

#if defined(PLAT_EIGEN) || defined(PLAT_EIGEN_718)
#define MP_THREAD_MIN_STACK_SIZE                        (4 * 1024)
#define MP_THREAD_DEFAULT_STACK_SIZE                    (MP_THREAD_MIN_STACK_SIZE + 1024 + 1024)
#define MP_THREAD_PRIORITY                              100
#else
#define MP_THREAD_MIN_STACK_SIZE                        (32 * 1024)
#define MP_THREAD_DEFAULT_STACK_SIZE                    (MP_THREAD_MIN_STACK_SIZE + 1024)
#define MP_THREAD_PRIORITY                              100
#endif


typedef Helios_Mutex_t mp_thread_mutex_t;
typedef Helios_Sem_t mp_thread_semphore_t;


#define QPY_WAIT_FOREVER    HELIOS_WAIT_FOREVER
#define QPY_NO_WAIT         HELIOS_NO_WAIT

void mp_thread_init(void *stack, uint32_t stack_len);
void mp_thread_gc_others(void);
void mp_thread_deinit(void);
unsigned int mp_get_available_memory_size(void);
bool mp_is_python_thread(void);
int mp_thread_get_current_tsknode(void);
int mp_thread_get_tskid_by_tsknode(void *th_node);

void mp_thread_mutex_init(mp_thread_mutex_t *mutex);
int mp_thread_mutex_lock(mp_thread_mutex_t *mutex, int wait);
void mp_thread_mutex_unlock(mp_thread_mutex_t *mutex);
void mp_thread_mutex_del(mp_thread_mutex_t *mutex);

void mp_thread_semphore_init(mp_thread_semphore_t *sem, uint32_t initcount);
int mp_thread_semphore_acquire(mp_thread_semphore_t *sem, int wait);
void mp_thread_semphore_release(mp_thread_semphore_t *sem);
void mp_thread_semphore_del(mp_thread_semphore_t *sem);

void mp_new_thread_add(uint32_t th_id, uint32_t stack_size);

#endif // MICROPY_INCLUDED_MPTHREADPORT_H

