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

#include "physical_mem.h"
#include "../../../../../common/arch/x86/kernel_image.h"
#include "../../../../setup/setup.h"
#include "../../../../debug/debug.h"
#include "../../../../rtl/rtl.h"

#define	PHY_PAGE_INFO(addr)	phy_mem_info[(addr)/4/1024]

static	u8		phy_mem_info[1024 * 1024];

static	void	print_phy_mem();
static	void	print_e820();

void setup_e820()
{
	u32 num;
	u32 i, base, end;
	pe820_table p_table;

	print_e820();

	//Anlyse e820
	dbg_print("Analyzing...\n");
	rtl_memset(phy_mem_info, 0, sizeof(phy_mem_info));
	num = **(u32**)KERNEL_MEM_INFO;

	for(p_table = (pe820_table)(*(u32**)KERNEL_MEM_INFO + 1), i = 0;
	    i < num && p_table->base_addr_h == 0;
	    i++, p_table++) {
		base = p_table->base_addr_l / 4096;
		end = (p_table->base_addr_l + p_table->len_l - 1) / 4096;

		while(base < end) {
			if(p_table->type == E820_USABLE) {
				if(phy_mem_info[base] != PHY_PAGE_RESERVED
				   && phy_mem_info[base] != PHY_PAGE_UNUSABLE
				   && phy_mem_info[base] != PHY_PAGE_SYSTEM) {
					if(base >= (KERNEL_BASE - VIRTUAL_ADDR_OFFSET) / 4096
					   && base <= (TMP_PAGE_TABLE_BASE + TMP_PAGE_SIZE) / 4096) {
						//Memory of kernel image
						phy_mem_info[base] = PHY_PAGE_SYSTEM;

					} else {
						//Free memories
						phy_mem_info[base] = PHY_PAGE_USABLE;
					}
				}

			} else {
				if(base < 1024 * 1024 / 4096) {
					//Reserved
					phy_mem_info[base] = PHY_PAGE_RESERVED;

				} else {
					//Unusable
					phy_mem_info[base] = PHY_PAGE_UNUSABLE;
				}
			}

			base++;
		}

	}

	//Set up memories which e820 didn't refered
	for(base = 0; base < 1024 * 1024; base++) {
		if(phy_mem_info[base] == 0) {
			if(base <= 1024 * 1024 / 4096) {
				//Reserved
				phy_mem_info[base] = PHY_PAGE_RESERVED;

			} else {
				//Unusable
				phy_mem_info[base] = PHY_PAGE_UNUSABLE;
			}
		}
	}

	print_phy_mem();

	return;
}

void print_e820()
{
	u32 i;
	pe820_table p_table;
	u32 num;

	dbg_print("Bios e820 info:\n");
	dbg_print("%-25s%-12s\n", "Range", "Type");

	num = **(u32**)KERNEL_MEM_INFO;

	for(p_table = (pe820_table)(*(u32**)KERNEL_MEM_INFO + 1), i = 0;
	    i < num && p_table->base_addr_h == 0;
	    i++, p_table++) {
		dbg_print("%-p-->%-12p",
		          p_table->base_addr_l,
		          (u32)((u64)(p_table->len_l) + p_table->base_addr_l - 1));

		switch(p_table->type) {
		case E820_USABLE:
			dbg_print("%-12s\n", "E820_USABLE");
			break;

		case E820_RESERVED:
			dbg_print("%-12s\n", "E820_RESERVED");
			break;

		default:
			dbg_print("%-12s\n", "Unknow");
		}

	}

	dbg_print("\n");
	return;
}

void print_phy_mem()
{
	u32 type;

	u32 i, base;
	dbg_print("Physical memory info:\n");
	dbg_print("%-25s%-12s\n", "Range", "Type");

	type = phy_mem_info[0];;
	base = 0;

	for(i = 0; i < 1024 * 1024; i++) {
		if(type != phy_mem_info[i] && i != 0) {
			dbg_print("%-p-->%-12p", base, i * 4096 - 1);

			switch(type) {
			case 0:
				dbg_print("%-12s\n", "0");
				break;

			case PHY_PAGE_RESERVED:
				dbg_print("%-12s\n", "PHY_PAGE_RESERVED");
				break;

			case PHY_PAGE_USABLE:
				dbg_print("%-12s\n", "PHY_PAGE_USABLE");
				break;

			case PHY_PAGE_UNUSABLE:
				dbg_print("%-12s\n", "PHY_PAGE_UNUSABLE");
				break;

			case PHY_PAGE_SYSTEM:
				dbg_print("%-12s\n", "PHY_PAGE_SYSTEM");
				break;

			case PHY_PAGE_ALLOCATED:
				dbg_print("%-12s\n", "PHY_PAGE_ALLOCATED");
				break;

			default:
				dbg_print("%-12s\n", "Unknow");
			}

			type = phy_mem_info[i];
			base = i * 4096;
		}
	}


	dbg_print("%-p-->%-12p", base, i * 4096 - 1);

	switch(type) {
	case PHY_PAGE_RESERVED:
		dbg_print("%-12s\n", "PHY_PAGE_RESERVED");
		break;

	case PHY_PAGE_USABLE:
		dbg_print("%-12s\n", "PHY_PAGE_USABLE");
		break;

	case PHY_PAGE_UNUSABLE:
		dbg_print("%-12s\n", "PHY_PAGE_UNUSABLE");
		break;

	case PHY_PAGE_SYSTEM:
		dbg_print("%-12s\n", "PHY_PAGE_SYSTEM");
		break;

	case PHY_PAGE_ALLOCATED:
		dbg_print("%-12s\n", "PHY_PAGE_ALLOCATED");
		break;

	default:
		dbg_print("%-12s\n", "Unknow");
	}

	dbg_print("\n");
	return;
}
