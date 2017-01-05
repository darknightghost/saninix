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

#include "../../../hal/exception/exception.h"

#include "../../rtl/rtl.h"
#include "../../mm/mm.h"
#include "../../pm/pm.h"

#include "./ebreakpoint_except.h"
#include "../exception.h"

pebreakpoint_except_t ebreakpoint_except()
{
    pebreakpoint_except_t p_ret = (pebreakpoint_except_t)except_obj(
                                      sizeof(ebreakpoint_except_t),
                                      EBREAKPOINT);

    if(p_ret != NULL) {
        p_ret->except.obj.class_id = CLASS_EXCEPT(EBREAKPOINT);
    }

    return p_ret;
}
