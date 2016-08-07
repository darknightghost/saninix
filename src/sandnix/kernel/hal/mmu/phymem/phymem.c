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

#include "phymem.h"
#include "../../init/init.h"
#include "../../early_print/early_print.h"
#include "../../rtl/rtl.h"
#include "../../exception/exception.h"
#include "../mmu.h"

#define	IN_RANGE(n, start, size) ((n) >= (start) && (n) < (start) + (size))

static	map_t			unusable_map;
static	map_t			free_map;
static	map_t			used_map;
static	map_t			dma_map;
static	bool			initialized = false;
static	pheap_t			phymem_heap;
static	u8				phymem_heap_block[4096];

static	bool			should_merge(plist_node_t p_pos1, plist_node_t p_pos2);
static	void			merge_mem(plist_node_t* p_p_pos1, plist_node_t* p_p_pos2,
                                  plist_t p_list);
static	int				compare_memblock(pphysical_memory_info_t p1,
        pphysical_memory_info_t p2);
static	void			print_mem_list(list_t list);
static	void			init_map(list_t mem_list);

void phymem_init()
{
    list_t info_list;
    plist_node_t p_pos1;
    plist_node_t p_pos2;
    pphysical_memory_info_t p_phy_mem;
    u64 offset;
    pphysical_memory_info_t p_system_info;
    void* initrd_addr;
    size_t initrd_size;
    address_t initrd_end;

    //Get physical memeory information from bootloader.
    info_list = hal_init_get_boot_memory_map();

    //Print boot informatiom
    hal_early_print_printf("\nPhysical memory infomation from bootloader:\n");
    print_mem_list(info_list);

    for(p_pos1 = info_list;
        p_pos1 != NULL;
        p_pos1 = core_rtl_list_next(p_pos1, &info_list)) {
        p_phy_mem = (pphysical_memory_info_t)(p_pos1->p_item);
        //Make the memory block SANDNIX_KERNEL_PAGE_SIZE aligned.
        offset = hal_rtl_math_mod64(p_phy_mem->begin, SANDNIX_KERNEL_PAGE_SIZE);

        if(offset > 0) {
            p_phy_mem->begin -= offset;
            p_phy_mem->size += offset;
        }

        offset = hal_rtl_math_mod64(p_phy_mem->size, SANDNIX_KERNEL_PAGE_SIZE);

        if(offset > 0) {
            p_phy_mem->size += SANDNIX_KERNEL_PAGE_SIZE - offset;
        }

    }

    hal_early_print_printf("\nAnalysing...\n");
    archecture_phyaddr_edit(&info_list);

    //Kernel
    p_system_info = core_mm_heap_alloc(sizeof(physical_memory_info_t), NULL);

    if(p_system_info == NULL) {
        hal_exception_panic(ENOMEM,
                            "Failed to allocate memory for physical memory information.");
    }

    p_system_info->begin = (u64)(address_t)(MIN(kernel_header.code_start,
                                            kernel_header.data_start) + get_load_pffset());
    p_system_info->size = (u64)(address_t)(MAX(kernel_header.code_start
                                           + kernel_header.code_size,
                                           kernel_header.data_start + kernel_header.data_size)
                                           + get_load_pffset() - p_system_info->begin);
    p_system_info->type = PHYMEM_SYSTEM;

    core_rtl_list_insert_after(NULL, &info_list, p_system_info, NULL);

    //Initrd
    hal_init_get_initrd_addr(&initrd_addr, &initrd_size);
    p_system_info = core_mm_heap_alloc(sizeof(physical_memory_info_t), NULL);

    if(p_system_info == NULL) {
        hal_exception_panic(ENOMEM,
                            "Failed to allocate memory for physical memory information.");
    }

    p_system_info->begin = (u64)((address_t)initrd_addr / 4096 * 4096);
    initrd_end = (address_t)initrd_addr + initrd_size;

    p_system_info->size = (u64)(initrd_end % 4096 > 0
                                ? (initrd_end / 4096 + 1) * 4096
                                : initrd_end) - p_system_info->begin;
    p_system_info->type = PHYMEM_SYSTEM;

    core_rtl_list_insert_after(NULL, &info_list, p_system_info, NULL);

    //Merg conflict memory blocks.
    p_pos1 = info_list;

    while(core_rtl_list_next(p_pos1, &info_list) != NULL) {
        p_pos2 = core_rtl_list_next(p_pos1, &info_list);

        while(true) {
            if(should_merge(p_pos1, p_pos2)) {
                merge_mem(&p_pos1, &p_pos2, &info_list);
                continue;

            } else {
                p_pos2 = core_rtl_list_next(p_pos2, &info_list);
            }

            if(p_pos2 == NULL) {
                p_pos1 = core_rtl_list_next(p_pos1, &info_list);
                break;
            }
        }
    }

    core_rtl_list_qsort(&info_list, (item_compare_t)compare_memblock, false);

    //Print the result
    hal_early_print_printf("\nPhysical memory infomation:\n");
    print_mem_list(info_list);

    //Create the heap
    phymem_heap = core_mm_heap_create_on_buf(
                      HEAP_MULITHREAD | HEAP_PREALLOC,
                      4096, phymem_heap_block, sizeof(phymem_heap_block));

    if(phymem_heap == NULL) {
        hal_exception_panic(ENOMEM,
                            "Not enough memory for physical memory managment.");
    }

    //Create maps
    init_map(info_list);

    //Destroy the list
    core_rtl_list_destroy(&info_list, NULL, (item_destroyer_t)core_mm_heap_free,
                          NULL);
    initialized = true;
    return;
}

kstatus_t hal_mmu_phymem_alloc(
    void** p_addr,		//Start address
    bool is_dma,		//DMA page
    size_t page_num);	//Num

void hal_mmu_phymem_free(
    void* addr,			//Address
    size_t page_num);	//Num

size_t hal_mmu_get_phymem_info(
    pphysical_memory_info_t p_buf,	//Pointer to buffer
    size_t size);

bool should_merge(plist_node_t p_pos1, plist_node_t p_pos2)
{
    pphysical_memory_info_t p1;
    pphysical_memory_info_t p2;

    if(p_pos1 == p_pos2) {
        return false;
    }

    p1 = (pphysical_memory_info_t)(p_pos1->p_item);
    p2 = (pphysical_memory_info_t)(p_pos2->p_item);

    if((p1->begin + p1->size == p2->begin
        || p2->begin + p2->size == p1->begin)
       && p1->type == p2->type) {
        return true;
    }

    if(IN_RANGE(p1->begin, p2->begin, p2->size)
       || IN_RANGE(p1->begin + p1->size - 1, p2->begin, p2->size)
       || IN_RANGE(p2->begin, p1->begin, p1->size)
       || IN_RANGE(p2->begin + p2->size - 1, p1->begin, p1->size)) {
        return true;

    } else {
        return false;
    }
}

void merge_mem(plist_node_t* p_p_pos1, plist_node_t* p_p_pos2, plist_t p_list)
{
    plist_node_t p_pos1;
    plist_node_t p_pos2;
    plist_node_t p_ret_pos1;
    plist_node_t p_ret_pos2;
    pphysical_memory_info_t p_1;
    pphysical_memory_info_t p_2;
    pphysical_memory_info_t p_new;

    u64 begin;
    u64 end;

    p_pos1 = *p_p_pos1;
    p_pos2 = *p_p_pos2;
    p_1 = (pphysical_memory_info_t)(p_pos1->p_item);
    p_2 = (pphysical_memory_info_t)(p_pos2->p_item);

    if(p_1->type > p_2->type) {
        //1>2
        p_ret_pos1 = p_pos1;

        if(p_1->begin + p_1->size > p_2->begin
           && p_2->begin + p_2->size > p_1->begin + p_1->size
           && p_1->begin < p_2->begin) {
            //End of pos1
            end = p_2->begin + p_2->size;
            p_2->begin = p_1->begin + p_1->size;
            p_2->size = end - p_2->begin;
            p_ret_pos2 = core_rtl_list_next(p_pos2, p_list);

        } else if(p_2->begin + p_2->size > p_1->begin
                  && p_1->begin + p_1->size > p_2->begin + p_2->size
                  && p_1->begin > p_2->begin) {
            //Begining of pos1
            p_2->size = p_1->begin - p_2->begin;
            p_ret_pos2 = core_rtl_list_next(p_pos2, p_list);

        } else if(p_1->begin <= p_2->begin
                  && p_1->begin + p_1->size >= p_2->begin + p_2->size) {
            //Pos1 contains pos2
            p_ret_pos2 = core_rtl_list_prev(p_pos2, p_list);
            core_rtl_list_remove(p_pos2, p_list, NULL);

            //Merge two memory blocks
            end = MAX(p_1->begin + p_1->size, p_2->begin + p_2->size);
            begin = MIN(p_1->begin, p_2->begin);
            p_1->begin = begin;
            p_1->size = end - begin;
            core_mm_heap_free(p_2, NULL);
            p_ret_pos2 = core_rtl_list_next(p_ret_pos2, p_list);

        } else {
            //Pos2 contains pos1
            p_ret_pos2 = core_rtl_list_prev(p_pos2, p_list);
            core_rtl_list_remove(p_pos2, p_list, NULL);

            p_new = core_mm_heap_alloc(sizeof(physical_memory_info_t), NULL);

            if(p_new == NULL) {
                hal_exception_panic(ENOMEM,
                                    "Failed to allocate memory for physical memory information.");
            }

            p_new->type = p_2->type;

            p_new->begin = p_1->begin + p_1->size;
            p_new->size = p_2->begin + p_2->size - p_new->begin;
            p_2->size = p_1->begin - p_2->begin;

            if(p_2->size == 0) {
                core_mm_heap_free(p_2, NULL);

            } else {
                core_rtl_list_insert_after(NULL, p_list, p_2, NULL);
            }

            if(p_new->size == 0) {
                core_mm_heap_free(p_new, NULL);

            } else {
                core_rtl_list_insert_after(NULL, p_list, p_new, NULL);
            }

            p_ret_pos2 = core_rtl_list_next(p_ret_pos2, p_list);
        }

    } else if(p_1->type == p_2->type) {
        p_ret_pos1 = p_pos1;

        //Remove p_pos2
        core_rtl_list_remove(p_pos2, p_list, NULL);

        //Merge two memory blocks
        end = MAX(p_1->begin + p_1->size, p_2->begin + p_2->size);
        begin = MIN(p_1->begin, p_2->begin);
        p_1->begin = begin;
        p_1->size = end - begin;
        core_mm_heap_free(p_2, NULL);
        p_ret_pos2 = core_rtl_list_next(p_pos1, p_list);

    } else {
        //2>1
        if(p_2->begin + p_2->size > p_1->begin
           && p_1->begin + p_1->size > p_2->begin + p_2->size
           && p_2->begin < p_1->begin) {
            //End of pos2
            end = p_1->begin + p_1->size;
            p_1->begin = p_2->begin + p_2->size;
            p_1->size = end - p_2->begin;
            p_ret_pos1 = p_pos1;
            p_ret_pos2 = core_rtl_list_next(p_pos2, p_list);

        } else if(p_1->begin + p_1->size > p_2->begin
                  && p_2->begin + p_2->size > p_1->begin + p_1->size
                  && p_2->begin > p_1->begin) {
            //Begining of pos2
            p_1->size = p_2->begin - p_1->begin;
            p_ret_pos1 = p_pos1;
            p_ret_pos2 = core_rtl_list_next(p_pos2, p_list);

        } else if(p_2->begin <= p_1->begin
                  && p_2->begin + p_2->size >= p_1->begin + p_1->size) {
            //Pos2 contains pos1
            p_ret_pos1 = core_rtl_list_prev(p_pos1, p_list);
            core_rtl_list_remove(p_pos1, p_list, NULL);

            //Merge two memory blocks
            end = MAX(p_1->begin + p_1->size, p_2->begin + p_2->size);
            begin = MIN(p_1->begin, p_2->begin);
            p_2->begin = begin;
            p_2->size = end - begin;
            core_mm_heap_free(p_1, NULL);

            if(p_ret_pos1 == NULL) {
                p_ret_pos1 = *p_list;

            } else {
                p_ret_pos1 = core_rtl_list_next(p_ret_pos1, p_list);
            }

            p_ret_pos2 = core_rtl_list_next(p_ret_pos1, p_list);

        } else {
            //Pos1 contains pos2
            p_ret_pos1 = core_rtl_list_prev(p_pos1, p_list);
            core_rtl_list_remove(p_pos1, p_list, NULL);

            p_new = core_mm_heap_alloc(sizeof(physical_memory_info_t), NULL);

            if(p_new == NULL) {
                hal_exception_panic(ENOMEM,
                                    "Failed to allocate memory for physical memory information.");
            }

            p_new->type = p_1->type;

            p_new->begin = p_2->begin + p_2->size;
            p_new->size = p_1->begin + p_1->size - p_new->begin;
            p_1->size = p_2->begin - p_1->begin;

            if(p_1->size == 0) {
                core_mm_heap_free(p_1, NULL);

            } else {
                core_rtl_list_insert_after(NULL, p_list, p_1, NULL);
            }

            if(p_new->size == 0) {
                core_mm_heap_free(p_new, NULL);

            } else {
                core_rtl_list_insert_after(NULL, p_list, p_new, NULL);
            }

            if(p_ret_pos1 == NULL) {
                p_ret_pos1 = *p_list;

            } else {
                p_ret_pos1 = core_rtl_list_next(p_ret_pos1, p_list);
            }

            p_ret_pos2 = core_rtl_list_next(p_ret_pos1, p_list);
        }

    }

    if(p_ret_pos1 == NULL) {
        *p_p_pos1 = *p_list;

    } else {
        *p_p_pos1 = p_ret_pos1;
    }

    if(p_ret_pos2 == NULL) {
        *p_p_pos2 = *p_list;

    } else {
        *p_p_pos2 = p_ret_pos2;
    }

    return;
}

int compare_memblock(pphysical_memory_info_t p1, pphysical_memory_info_t p2)
{
    if(p1->begin > p2->begin) {
        return 1;

    } else if(p1->begin == p2->begin) {
        return 0;

    } else {
        return -1;
    }
}

void print_mem_list(list_t list)
{
    char* type_str;
    plist_node_t p_pos;
    pphysical_memory_info_t p_phy_mem;

    hal_early_print_printf("%-20s%-20s%s\n", "Base", "Size", "Type");

    for(p_pos = list;
        p_pos != NULL;
        p_pos = core_rtl_list_next(p_pos, &list)) {
        p_phy_mem = (pphysical_memory_info_t)(p_pos->p_item);

        switch(p_phy_mem->type) {
            case PHYMEM_AVAILABLE:
                type_str = "PHYMEM_AVAILABLE";
                break;

            case PHYMEM_DMA:
                type_str = "PHYMEM_DMA";
                break;

            case PHYMEM_USED:
                type_str = "PHYMEM_USED";
                break;

            case PHYMEM_DMA_USED:
                type_str = "PHYMEM_DMA_USED";
                break;

            case PHYMEM_SYSTEM:
                type_str = "PHYMEM_SYSTEM";
                break;

            case PHYMEM_RESERVED:
                type_str = "PHYMEM_RESERVED";
                break;

            case PHYMEM_BAD:
                type_str = "PHYMEM_BAD";
                break;
        }

        hal_early_print_printf("0x%-18.16llX0x%-18.16llX%s\n", p_phy_mem->begin,
                               p_phy_mem->size, type_str);

    }

    return;
}

void init_map(list_t mem_list)
{
    plist_node_t p_node;
    pphysical_memory_info_t p_phymem;
    pphysical_memory_info_t p_new_phymem;

    //Initialize maps
    core_rtl_map_init(&unusable_map, (item_compare_t)compare_memblock,
                      phymem_heap);
    core_rtl_map_init(&free_map, (item_compare_t)compare_memblock,
                      phymem_heap);
    core_rtl_map_init(&used_map, (item_compare_t)compare_memblock,
                      phymem_heap);
    core_rtl_map_init(&dma_map, (item_compare_t)compare_memblock,
                      phymem_heap);

    //Add memory blocks to the map
    p_node = mem_list;

    do {
        p_phymem = (pphysical_memory_info_t)(p_node->p_item);
        p_new_phymem = core_mm_heap_alloc(sizeof(physical_memory_info_t),
                                          phymem_heap);

        if(p_new_phymem == NULL) {
            hal_exception_panic(ENOMEM,
                                "Failed to allocate memory for physical memory "
                                "managment module.");
        }

        core_rtl_memcpy(p_new_phymem, p_phymem, sizeof(physical_memory_info_t));

        switch(p_new_phymem->type) {
            case PHYMEM_AVAILABLE:
                if(core_rtl_map_set(&free_map, p_new_phymem, p_new_phymem)
                   == NULL) {
                    hal_exception_panic(ENOMEM,
                                        "Failed to allocate memory for physical memory "
                                        "managment module.");
                }

                break;

            case PHYMEM_DMA:
                if(core_rtl_map_set(&dma_map, p_new_phymem, p_new_phymem)
                   == NULL) {
                    hal_exception_panic(ENOMEM,
                                        "Failed to allocate memory for physical memory "
                                        "managment module.");
                }

                break;

            case PHYMEM_USED:
            case PHYMEM_DMA_USED:
                if(core_rtl_map_set(&used_map, p_new_phymem, p_new_phymem)
                   == NULL) {
                    hal_exception_panic(ENOMEM,
                                        "Failed to allocate memory for physical memory "
                                        "managment module.");
                }

                break;

            case PHYMEM_SYSTEM:
            case PHYMEM_RESERVED:
            case PHYMEM_BAD:
                if(core_rtl_map_set(&unusable_map, p_new_phymem, p_new_phymem)
                   == NULL) {
                    hal_exception_panic(ENOMEM,
                                        "Failed to allocate memory for physical memory "
                                        "managment module.");
                }

                break;

            default:
                hal_exception_panic(EINVAL,
                                    "Illegal physical memory type :\"%u\"."
                                    , p_new_phymem->type);

        }

        p_node = core_rtl_list_next(p_node, &mem_list);
    } while(p_node != NULL);

    return;
}
