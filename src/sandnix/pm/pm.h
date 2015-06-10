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

#ifndef	PM_H_INCLUDE
#define	PM_H_INCLUDE

#include "../common/common.h"

#ifdef	X86
	#include "arch/x86/schedule.h"
#endif	//X86

#include "mutex/mutex.h"
#include "semaphore/semaphore.h"
#include "spinlock/arch/x86/spinlock.h"

#define		MAX_PROCESS_NUM		65535

//Thread function
//void			thread_func(u32 thread_id,void* p_args);
typedef	void	(*thread_func)(u32, void*);

//All
void		pm_init();

//Thread
void		pm_schedule();
u32			pm_create_thrd(void* entry, bool is_user_thread, void* p_args);
void		pm_terminate_thrd(u32 thread_id, u32 exit_code);
void		pm_suspend_thrd(u32 thread_id);
void		pm_resume_thrd(u32 thread_id);
void		pm_sleep(u32 ms);
u32			pm_get_crrnt_thrd_id();
void		pm_task_schedule();

bool		pm_get_thread_context(u32 id, pcontext p_cntxt);
bool		pm_set_thread_context(u32 id, pcontext p_cntxt);

//Process
u32			pm_fork();
void		pm_exit(u32 exit_code);
u32			pm_wait(u32 thread_id);
void		pm_exec(char* cmd_line);
u32			pm_get_pdt_id(u32 process_id);
u32			pm_get_proc_id(u32 thread_id);

//Spin lock
void		pm_init_spn_lock(pspin_lock p_lock);
void		pm_acqr_spn_lock(pspin_lock p_lock);
bool		pm_try_acqr_spn_lock(pspin_lock p_lock);
void		pm_rls_spn_lock(pspin_lock p_lock);

//Mutex
void		pm_init_mutex(pmutex p_mutex);
void		pm_acqr_mutex(pmutex p_mutex);
bool		pm_try_acqr_mutex(pmutex p_mutex);
void		pm_rls_mutex(pmutex p_mutex);

//Semaphore
void		pm_init_semaphore(psemaphore p_semaphore);
void		pm_acqr_semaphore(psemaphore p_semaphore);
bool		pm_try_acqr_semaphore(psemaphore p_semaphore);
void		pm_rls_semaphore(psemaphore p_semaphore);

#endif	//!	PM_H_INCLUDE

