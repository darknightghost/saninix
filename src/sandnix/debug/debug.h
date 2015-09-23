/*
	Copyright 2015,暗夜幽灵 <darknightghost.cn@gmail.com>

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

#ifndef	DEBUG_H_INCLUDE
#define	DEBUG_H_INCLUDE

#include "../../common/common.h"
#include "../exceptions/exceptions.h"

#define	K_TTY_BUF_SIZE		4096

#ifdef DEBUG

#define	ASSERT(x)	{\
		if(!(x)){\
			excpt_panic(EASSERT,\
			            "Assert failed.\nExpression:%s\nFile:%s\nLine:%u",\
			            #x,\
			            __FILE__,\
			            __LINE__);\
		}\
	}
#endif	//	DEBUG
#ifndef DEBUG
	#define	ASSERT(x)
#endif	//!	DEBUG

void	dbg_init();
void	dbg_cls();
void	dbg_print(char* fmt, ...);

#endif	//!	DEBUG_H_INCLUDE