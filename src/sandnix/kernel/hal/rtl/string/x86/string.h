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

#include "../../../../../../common/common.h"

#define hal_rtl_string_movsb(dest, src, count) { \
        do {\
            __asm__ __volatile__( \
                                  "cld\n" \
                                  "movl	%0, %%edi\n" \
                                  "movl	%1, %%esi\n" \
                                  "rep	movsb\n" \
                                  ::"m"((u32)(dest)), "m"((u32)(src)), \
                                  "ecx"((u32)(count)) \
                                  :"memory", "esi", "edi"); \
        } while(0); \
    }

#define hal_rtl_string_movsw(dest, src, count) { \
        do {\
            __asm__ __volatile__( \
                                  "cld\n" \
                                  "movl	%0, %%edi\n" \
                                  "movl	%1, %%esi\n" \
                                  "rep	movsw\n" \
                                  ::"m"((u32)(dest)), "m"((u32)(src)), \
                                  "ecx"((u32)(count)) \
                                  :"memory", "esi", "edi"); \
        } while(0); \
    }


#define hal_rtl_string_movsl(dest, src, count) { \
        do {\
            __asm__ __volatile__( \
                                  "cld\n" \
                                  "movl	%0, %%edi\n" \
                                  "movl	%1, %%esi\n" \
                                  "rep	movsl\n" \
                                  ::"m"((u32)(dest)), "m"((u32)(src)), \
                                  "ecx"((u32)(count)) \
                                  :"memory", "esi", "edi"); \
        } while(0); \
    }

#define hal_rtl_string_movsq(dest, src, count) { \
        do {\
            __asm__ __volatile__( \
                                  "cld\n" \
                                  "movl	%0, %%edi\n" \
                                  "movl	%1, %%esi\n" \
                                  "rep	movsl\n" \
                                  ::"m"((u32)(dest)), "m"((u32)(src)), \
                                  "ecx"((u32)(count * 2)) \
                                  :"memory", "esi", "edi"); \
        } while(0); \
    }

#define hal_rtl_string_movsb_back(dest, src, count) { \
        do {\
            __asm__ __volatile__( \
                                  "std\n" \
                                  "movl	%0, %%edi\n" \
                                  "movl	%1, %%esi\n" \
                                  "movl	%2, %%eax\n" \
                                  "decl	%%eax\n" \
                                  "addl	%%eax, %%edi\n" \
                                  "addl	%%eax, %%esi\n" \
                                  "rep	movsb\n" \
                                  ::"m"((u32)(dest)), "m"((u32)(src)), \
                                  "ecx"((u32)(count)) \
                                  :"eax", "edx", "esi", "edi", "memory"); \
        } while(0); \
    }

#define hal_rtl_string_movsw_back(dest, src, count) { \
        do {\
            __asm__ __volatile__( \
                                  "std\n" \
                                  "movl	%0, %%edi\n" \
                                  "movl	%1, %%esi\n" \
                                  "movl	%2, %%eax\n" \
                                  "decl	%%eax\n" \
                                  "mull	$2\n" \
                                  "addl	%%eax, %%edi\n" \
                                  "addl	%%eax, %%esi\n" \
                                  "rep	movsw\n" \
                                  ::"m"((u32)(dest)), "m"((u32)(src)), \
                                  "ecx"((u32)(count)) \
                                  :"eax", "edx", "esi", "edi", "memory"); \
        } while(0); \
    }


#define hal_rtl_string_movsl_back(dest, src, count) { \
        do {\
            __asm__ __volatile__( \
                                  "std\n" \
                                  "movl	%0, %%edi\n" \
                                  "movl	%1, %%esi\n" \
                                  "movl	%2, %%eax\n" \
                                  "decl	%%eax\n" \
                                  "mull	$4\n" \
                                  "addl	%%eax, %%edi\n" \
                                  "addl	%%eax, %%esi\n" \
                                  "rep	movsl\n" \
                                  ::"m"((u32)(dest)), "m"((u32)(src)), \
                                  "ecx"((u32)(count)) \
                                  :"eax", "edx", "esi", "edi", "memory"); \
        } while(0); \
    }

#define hal_rtl_string_movsq_back(dest, src, count) { \
        do {\
            __asm__ __volatile__( \
                                  "std\n" \
                                  "movl	%0, %%edi\n" \
                                  "movl	%1, %%esi\n" \
                                  "movl	%2, %%eax\n" \
                                  "decl	%%eax\n" \
                                  "mull	$4\n" \
                                  "addl	%%eax, %%esi\n" \
                                  "addl	%%eax, %%edi\n" \
                                  "rep	movsl\n" \
                                  ::"m"((u32)(dest)), "m"((u32)(src)), \
                                  "ecx"((u32)(count * 2)) \
                                  :"eax", "edx", "esi", "edi", "memory"); \
        } while(0); \
    }
