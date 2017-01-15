/*
    Copyright 2016,王思远 <darknightghost.cn@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "../../../../common/common.h"

#include "thread_except_stat_obj.h"
#include "except_obj/except_obj.h"

#include "./exception_defs.h"
#include "../../hal/exception/exception.h"

//Initialize module
void core_exception_init();

pthread_except_stat_obj_t	core_exception_get_0();

//Set errno
void core_exception_set_errno(kstatus_t status);

//Get errno
kstatus_t core_exception_get_errno();

//Raise exception
void core_exception_raise(pexcept_obj_t except);

//Regist global exception handler
plist_node_t core_exception_add_hndlr(kstatus_t reason, except_hndlr_t hndlr);

//Unregist global exception handler
void core_exception_remove_hndlr(plist_node_t pos);

//Push exception hndlr of current thread
//except_ret_stat_t core_exception_push_hndlr(pexcept_hndlr_info_t p_hndlr_info);
#define core_exception_push_hndlr(p_hndlr_info);

//Pop exception hndlr of current thread
pexcept_hndlr_info_t core_exception_pop_hndlr();
