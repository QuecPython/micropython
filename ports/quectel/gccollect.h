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

#ifndef __MICROPY_INCLUDED_QUECTEL_GCCOLLECT_H__
#define __MICROPY_INCLUDED_QUECTEL_GCCOLLECT_H__

void gc_stacktop_set(void * ptr);
void gc_collect(void);

//Set a stack address as the end address of the GC collection
//Do not add {} to the macro definition to prevent the scoping of stack_dummy from changing
//注意后面括号不能加大括号，防止stack_dummy的作用域发生变更。
#define GC_STACKTOP_SET()  int stack_dummy;gc_stacktop_set(&stack_dummy);

//Clears the gc collection end stack pointer for global non-Python threads
#define GC_STACKTOP_CLEAR() gc_stacktop_set(NULL);

#endif

