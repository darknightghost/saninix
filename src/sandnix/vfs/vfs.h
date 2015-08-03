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

#ifndef	VFS_H_INCLUDE
#define	VFS_H_INCLUDE

#include "../common/common.h"

void			vfs_init();

//File system
bool			vfs_reg_filesystem();
bool			vfs_unreg_filesystem();

//Devices
k_status		vfs_mount();
k_status		vfs_mount_root();
k_status		vfs_umount();

//Files
u32				vfs_open();
bool			vfs_inc_fdesc_reference(u32 file_descriptor);
k_status		vfs_chmod();
bool			vfs_access();
void			vfs_close();
size_t			vfs_read();
size_t			vfs_write();
void			vfs_sync();

#endif	//!	VFS_H_INCLUDE