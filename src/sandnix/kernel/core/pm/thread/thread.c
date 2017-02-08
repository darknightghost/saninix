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

#include "thread.h"
#include "./thread_obj.h"
#include "../../rtl/rtl.h"
#include "../../mm/mm.h"
#include "../../exception/exception.h"
#include "../../../hal/cpu/cpu.h"
#include "../../../hal/io/io.h"
#include "../pm.h"

//Flag
static	bool				initialized = false;

//Heap
static	pheap_t				sched_heap;

//Schedule list
static	list_t				sched_lists[PRIORITY_HIGHEST + 1];
static	spnlck_t			sched_list_lock;

//CPU status
static	core_sched_info_t	cpu_infos[MAX_CPU_NUM];

//Thread table
static	array_t				thread_table;
static	spnlck_rw_t			thread_table_lock;

static int tmp_priority = PRIORITY_DISPATCH;

static	void				on_tick(u32 int_num, pcontext_t p_context,
                                    u32 err_code);

static	void				add_use_count(pcore_sched_info_t p_info,
        bool is_busy);
static inline void			switch_task(pcore_sched_info_t p_info,
                                        plist_node_t p_node);

void core_pm_thread_init()
{
    //Initialize heap
    sched_heap = core_mm_heap_create(HEAP_MULITHREAD, SANDNIX_KERNEL_PAGE_SIZE);

    if(sched_heap == NULL) {
        PANIC(ENOMEM, "Failed to create schedule heap.");
    }

    //Initialize thread table
    core_rtl_array_init(&thread_table, MAX_THREAD_NUM, sched_heap);
    core_pm_spnlck_rw_init(&thread_table_lock);

    //Create thread 0
    pthread_obj_t p_thread_obj = thread_obj_0();
    p_thread_obj->status = TASK_RUNNING;

    if(p_thread_obj == NULL) {
        PANIC(ENOMEM, "Faile to create thread object for thread 0.");
    }

    core_rtl_array_set(&thread_table, 0, p_thread_obj);

    pcore_sched_info_t p_info = &cpu_infos[0];
    p_info->cpu_use_stat_l = 0;
    p_info->cpu_use_stat_h = 0;
    p_info->current_node = (plist_node_t)core_mm_heap_alloc(sizeof(list_node_t),
                           sched_heap);
    p_info->current_node->p_item = p_thread_obj;
    p_info->priority = PRIORITY_DISPATCH;
    p_info->idle_thread_id = 0;
    core_pm_spnlck_init(&(p_info->lock));

    //Initialize schedule list
    core_rtl_memset(sched_lists, 0, sizeof(sched_lists));
    core_rtl_memset(cpu_infos, 0, sizeof(cpu_infos));
    core_pm_spnlck_init(&sched_list_lock);

    //Set schedule callback
    hal_io_int_callback_set(INT_TICK, on_tick);
    initialized = true;
    return;
}

void core_pm_thread_core_init()
{
    u32 cpu_index = hal_cpu_get_cpu_index();
    pcore_sched_info_t p_info = &cpu_infos[cpu_index];
    p_info->cpu_use_stat_l = 0;
    p_info->cpu_use_stat_h = 0;
    p_info->current_node = NULL;
    core_pm_spnlck_init(&(p_info->lock));
    p_info->enabled = true;

    return;
}

void core_pm_thread_core_release()
{
    u32 cpu_index = hal_cpu_get_cpu_index();
    pcore_sched_info_t p_info = &cpu_infos[cpu_index];
    core_pm_spnlck_lock(&(p_info->lock));
    p_info->enabled = false;

    if(p_info->current_node != NULL) {
        //Get node
        plist_node_t p_node = p_info->current_node;
        p_info->current_node = NULL;
        core_pm_spnlck_unlock(&(p_info->lock));

        //Add Task to schedule list
        core_pm_spnlck_lock(&sched_list_lock);
        pthread_obj_t p_obj = (pthread_obj_t)(p_node->p_item);
        plist_t p_list = &sched_lists[p_obj->priority];

        if(*p_list == NULL) {
            *p_list = p_node;
            p_node->p_prev = p_node;
            p_node->p_next = p_node;

        } else {
            p_node->p_prev = (*p_list)->p_prev;
            p_node->p_next = (*p_list);
            (*p_list)->p_prev->p_next = p_node;
            (*p_list)->p_next->p_prev = p_node;
            *p_list = p_node;
        }

        core_pm_spnlck_unlock(&sched_list_lock);

    } else {
        core_pm_spnlck_unlock(&(p_info->lock));
    }

    return;
}

u32 core_pm_get_crrnt_thread_id()
{
    return 0;
}

u32 core_pm_get_thrd_priority(u32 thrd_id)
{
    UNREFERRED_PARAMETER(thrd_id);
    return tmp_priority;
}

void core_pm_set_thrd_priority(u32 thrd_id, u32 priority)
{
    UNREFERRED_PARAMETER(thrd_id);
    tmp_priority = priority;
    return;
}

void on_tick(u32 int_num, pcontext_t p_context, u32 err_code)
{
    //Get schedule info
    u32 cpu_index = hal_cpu_get_cpu_index();
    pcore_sched_info_t p_info = &cpu_infos[cpu_index];

    if(!p_info->enabled || p_info->priority == PRIORITY_HIGHEST) {
        hal_io_irq_send_eoi();
        hal_cpu_context_load(p_context);
    }

    pthread_obj_t p_thread_obj = NULL;

    if(p_info->current_node != NULL) {
        p_thread_obj = (pthread_obj_t)(p_info->current_node->p_item);
    }

    p_thread_obj->p_context = p_context;


    UNREFERRED_PARAMETER(int_num);
    UNREFERRED_PARAMETER(err_code);
    return;
}

void add_use_count(pcore_sched_info_t p_info, bool is_busy)
{
    core_pm_spnlck_raw_lock(&(p_info->lock));
    p_info->cpu_use_stat_h <<= 1;

    if(p_info->cpu_use_stat_l & 0x8000000000000000) {
        p_info->cpu_use_stat_h += 0x01;
    }

    p_info->cpu_use_stat_l <<= 1;

    if(is_busy) {
        p_info->cpu_use_stat_l += 1;
    }

    core_pm_spnlck_raw_unlock(&(p_info->lock));
    return;
}

void switch_task(pcore_sched_info_t p_info, plist_node_t p_node)
{
    //Add current thread to schedule list
    if(p_info->current_node != NULL) {
        if(sched_lists[p_info->priority] == NULL) {
            p_info->current_node->p_prev = NULL;
            p_info->current_node->p_next = NULL;
            sched_lists[p_info->priority] = NULL;

        } else {
            p_info->current_node->p_prev = sched_lists[p_info->priority]
                                           ->p_prev;
            p_info->current_node->p_next = sched_lists[p_info->priority];
            p_info->current_node->p_prev->p_next = p_info->current_node;
            p_info->current_node->p_next->p_prev = p_info->current_node;
        }
    }

    //Switch task
    p_info->current_node = p_node;

    pthread_obj_t p_thread_obj = (pthread_obj_t)(p_info->current_node->p_item);
    p_info->priority = p_thread_obj->priority;

    //Load context
    hal_cpu_context_load(p_thread_obj->p_context);

    return;
}
