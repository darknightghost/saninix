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

#pragma once

#include "../../../../common/common.h"
#include "../../pm/pm.h"

#define	HEAP_INTACT_MAGIC			(*((u32*)"HEAP"))
#define	KERNEL_DEFAULT_HEAP_SIZE	(4096*4)

#define		HEAP_EXTENDABLE				0x01
#define		HEAP_MULTITHREAD			0x02
#define		HEAP_DESTROY				0x04
#define		HEAP_PREALLOC				0x08

#pragma pack(push)
#ifdef	X86
	#pragma pack(4)
#endif	//X86

typedef	struct _mem_block_head {
	u32							magic;
	struct _mem_block_head*		p_prev_empty_block;
	struct _mem_block_head*		p_next_empty_block;
	u8*							start_addr;
	int							allocated_flag;
	size_t						size;
} mem_block_head_t, *pmem_block_head_t;

typedef	struct _heap_head {
	struct _heap_head*			p_prev;
	struct _heap_head*			p_next;
	pmem_block_head_t			p_first_empty_block;
	size_t						scale;
	u32							attr;
	spinlock_t					lock;
} heap_head_t, *pheap_head_t;

void*				mm_hp_create(size_t max_block_size, u32 attr);
void*				mm_hp_create_on_buf(void* buf, size_t buf_size, u32 attr);
void				mm_hp_destroy(void* heap_addr);
void*				mm_hp_alloc(size_t size, void* heap_addr);
void				mm_hp_chk(void* heap_addr);
void				mm_hp_free(void* addr, void* heap_addr);

void*				hp_alloc_mm(size_t size, void* heap_addr);

#pragma pack(pop)