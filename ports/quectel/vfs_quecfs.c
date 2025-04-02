/*
 * Copyright (c) Quectel Wireless Solution, Co., Ltd.All Rights Reserved.
 *  
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  
 *     http://www.apache.org/licenses/LICENSE-2.0
 *  
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <string.h>

#include "py/runtime.h"
#include "py/stream.h"
#include "py/binary.h"
#include "py/objarray.h"
#include "py/objstr.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/mpthread.h"
#include "extmod/vfs.h"

#if MICROPY_VFS_QUECFS
#include "vfs_quecfs.h"
#include "helios_fs.h"

enum { QUECFS_MAKE_ARG_pname };

const mp_arg_t quecfs_make_allowed_args[] = {   
    { MP_QSTR_pname, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
};

const char *mp_vfs_quecfs_make_path(mp_obj_vfs_quecfs_t *self, mp_obj_t path_in) {
	const char *path = mp_obj_str_get_str(path_in);
    if (path[0] != '/') {
        size_t l = vstr_len(&self->cur_dir);
        if (l > 0) {
            vstr_add_str(&self->cur_dir, path);
            path = vstr_null_terminated_str(&self->cur_dir);
            self->cur_dir.len = l;
        }
    }
    return path;
}

static mp_import_stat_t mp_vfs_quecfs_import_stat(void *self_in, const char *path) {
	mp_obj_vfs_quecfs_t *self = self_in;
    Helios_fstat_t file_stat = {0};
	
	
	//compatible for "from usr import module" function
	if(0 == strncmp(path, "usr", 3))
	{
		path = &path[3];
	}

	mp_obj_str_t path_obj = { { &mp_type_str }, 0, 0, (const byte *)path };
    path = mp_vfs_quecfs_make_path(self, MP_OBJ_FROM_PTR(&path_obj));
	char f_path[64] = "/usr";
	sprintf(f_path, "%s%s", "/usr", path);
	
	int ret = Helios_fstat(f_path, &file_stat);

	if (ret == 0) 
	{
        if(file_stat.f_isdir)
        {
            return MP_IMPORT_STAT_DIR;
        }
        else
        {
            return MP_IMPORT_STAT_FILE;
        }
	}

    return MP_IMPORT_STAT_NO_EXIST;
}

static mp_obj_t vfs_quecfs_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) 
{
	mp_arg_val_t args_parse[MP_ARRAY_SIZE(quecfs_make_allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(quecfs_make_allowed_args), quecfs_make_allowed_args, args_parse);
	char *partition_name = (char *)mp_obj_str_get_str(args_parse[QUECFS_MAKE_ARG_pname].u_obj);

    mp_obj_vfs_quecfs_t *vfs = m_new_obj(mp_obj_vfs_quecfs_t);
    vfs->base.type = type;
	vstr_init(&vfs->cur_dir, 16);
    vstr_add_byte(&vfs->cur_dir, '/');
	
	if(strcmp(partition_name, "customer_fs") == 0)
	{
		vfs->readonly = false;
		//create dir for micropython dir "qpy/usr"
		Helios_mkdir("/usr", 0777);
	}
	else if(strcmp(partition_name, "customer_backup_fs") == 0)
	{
		vfs->readonly = true;
		//create dir for micropython dir "qpy/bak"
		Helios_mkdir("/bak", 0777);
	}
    else
    {
        mp_raise_OSError(-MP_ENOENT);
    }
	
    return MP_OBJ_FROM_PTR(vfs);
}

static mp_obj_t vfs_quecfs_mount(mp_obj_t self_in, mp_obj_t readonly, mp_obj_t mkfs) 
{
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(vfs_quecfs_mount_obj, vfs_quecfs_mount);

static mp_obj_t vfs_quecfs_umount(mp_obj_t self_in) {
    (void)self_in;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(vfs_quecfs_umount_obj, vfs_quecfs_umount);

static mp_obj_t vfs_quecfs_open(mp_obj_t self_in, mp_obj_t path_in, mp_obj_t mode_in) {
    mp_obj_vfs_quecfs_t *self = MP_OBJ_TO_PTR(self_in);
    const char *mode = mp_obj_str_get_str(mode_in);
	const char *path = mp_obj_str_get_str(path_in);
    const mp_obj_type_t *io_type = &mp_type_vfs_quecfs_textio;

	//compatible for "from usr import module" function
	if(0 == strncmp(path, "usr", 3))
	{
		path = &path[3];
	}
	
	mp_obj_str_t path_obj = { { &mp_type_str }, 0, 0, (const byte *)path };
    path = mp_vfs_quecfs_make_path(self, MP_OBJ_FROM_PTR(&path_obj));
	path_obj.data = (const byte *)path;

    if(NULL != strchr(mode, 'b')) {
        io_type = &mp_type_vfs_quecfs_fileio;
    }

    return mp_vfs_quecfs_file_open(self_in, io_type, MP_OBJ_FROM_PTR(&path_obj), mode_in);
}
static MP_DEFINE_CONST_FUN_OBJ_3(vfs_quecfs_open_obj, vfs_quecfs_open);

static mp_obj_t vfs_quecfs_chdir(mp_obj_t self_in, mp_obj_t path_in) {
    mp_obj_vfs_quecfs_t *self = MP_OBJ_TO_PTR(self_in);

    // Check path exists
    const char *path = mp_vfs_quecfs_make_path(self, path_in);
    if (path[1] != '\0') {
        // Not at root, check it exists
	    Helios_fstat_t file_stat = {0};
		char f_path[64] = "";
		if(self->readonly)
		{
			sprintf(f_path, "%s%s", "/bak", path);
		}
		else
		{
			sprintf(f_path, "%s%s", "/usr", path);
		}
		
		int ret = Helios_fstat(f_path, &file_stat);
		if (ret != 0) 
        {
	        mp_raise_OSError(-MP_ENOENT);
	    }
    }

    // Update cur_dir with new path
    if (path == vstr_str(&self->cur_dir)) {
        self->cur_dir.len = strlen(path);
    } else {
        vstr_reset(&self->cur_dir);
        vstr_add_str(&self->cur_dir, path);
    }

    // If not at root add trailing / to make it easy to build paths
    // and then normalise the path
    if (vstr_len(&self->cur_dir) != 1) {
        vstr_add_byte(&self->cur_dir, '/');

	#define CWD_LEN (vstr_len(&self->cur_dir))
        size_t to = 1;
        size_t from = 1;
        char *cwd = vstr_str(&self->cur_dir);
        while (from < CWD_LEN) {
            for (; cwd[from] == '/' && from < CWD_LEN; ++from) {
                // Scan for the start
            }
            if (from > to) {
                // Found excessive slash chars, squeeze them out
                vstr_cut_out_bytes(&self->cur_dir, to, from - to);
                from = to;
            }
            for (; cwd[from] != '/' && from < CWD_LEN; ++from) {
                // Scan for the next /
            }
            if ((from - to) == 1 && cwd[to] == '.') {
                // './', ignore
                vstr_cut_out_bytes(&self->cur_dir, to, ++from - to);
                from = to;
            } else if ((from - to) == 2 && cwd[to] == '.' && cwd[to + 1] == '.') {
                // '../', skip back
                if (to > 1) {
                    // Only skip back if not at the tip
                    for (--to; to > 1 && cwd[to - 1] != '/'; --to) {
                        // Skip back
                    }
                }
                vstr_cut_out_bytes(&self->cur_dir, to, ++from - to);
                from = to;
            } else {
                // Normal element, keep it and just move the offset
                to = ++from;
            }
        }
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(vfs_quecfs_chdir_obj, vfs_quecfs_chdir);

static mp_obj_t vfs_quecfs_getcwd(mp_obj_t self_in) {
    mp_obj_vfs_quecfs_t *self = MP_OBJ_TO_PTR(self_in);
    if (vstr_len(&self->cur_dir) == 1) {
        return MP_OBJ_NEW_QSTR(MP_QSTR__slash_);
    } else {
        // don't include trailing /
        return mp_obj_new_str(self->cur_dir.buf, self->cur_dir.len - 1);
    }
}
static MP_DEFINE_CONST_FUN_OBJ_1(vfs_quecfs_getcwd_obj, vfs_quecfs_getcwd);

typedef struct _vfs_quecfs_ilistdir_it_t {
    mp_obj_base_t base;
    mp_fun_1_t iternext;
    bool is_str;
    void *dir;
	char ilist_path[64];
	bool readonly;
} vfs_quecfs_ilistdir_it_t;

static mp_obj_t vfs_quecfs_ilistdir_it_iternext(mp_obj_t self_in) {
    vfs_quecfs_ilistdir_it_t *self = MP_OBJ_TO_PTR(self_in);
	char real_file_path[128] = "";
	Helios_fstat_t file_stat = {0};

    if (self->dir == NULL) 
	{
        return MP_OBJ_STOP_ITERATION;
    }
	HeliosDIR_Iter iter_entry = {0};
	
	for (;;) 
	{
		int ret = Helios_readdir(self->dir, &iter_entry);
		
        if (ret != 0 || strlen(iter_entry.d_name) == 0) 
		{
			// stop on error or end of dir
			break;
        }

		if (iter_entry.d_name[0] == '.' && (iter_entry.d_name[1] == 0 || iter_entry.d_name[1] == '.')) 
		{
            // skip . and ..
            continue;
        }	   

		// make 4-tuple with info about this entry
	    mp_obj_tuple_t *t = MP_OBJ_TO_PTR(mp_obj_new_tuple(4, NULL));
	    if (self->is_str) 
		{
	        t->items[0] = mp_obj_new_str(iter_entry.d_name, strlen(iter_entry.d_name));
	    } 
		else 
		{
	        t->items[0] = mp_obj_new_bytes((const byte *)iter_entry.d_name, strlen(iter_entry.d_name));
	    }

		sprintf(real_file_path, "%s/%s", self->ilist_path, iter_entry.d_name);
		Helios_fstat(real_file_path, &file_stat);
		memset (real_file_path, 0, sizeof(real_file_path));
	    t->items[1] = MP_OBJ_NEW_SMALL_INT(file_stat.f_isdir ? MP_S_IFDIR : MP_S_IFREG);
	    t->items[2] = MP_OBJ_NEW_SMALL_INT(0); // no inode number
	    t->items[3] = MP_OBJ_NEW_SMALL_INT(file_stat.f_size);

		return MP_OBJ_FROM_PTR(t);   
    }

	Helios_closedir(self->dir);
	self->dir = NULL;
	
	return MP_OBJ_STOP_ITERATION;
}

static mp_obj_t vfs_quecfs_ilistdir(mp_obj_t self_in, mp_obj_t path_in) 
{
	mp_obj_vfs_quecfs_t *self = MP_OBJ_TO_PTR(self_in);
    vfs_quecfs_ilistdir_it_t *iter = m_new_obj(vfs_quecfs_ilistdir_it_t);
    iter->base.type = &mp_type_polymorph_iter;
    iter->iternext = vfs_quecfs_ilistdir_it_iternext;
    iter->is_str = mp_obj_get_type(path_in) == &mp_type_str;
    const char *path = mp_obj_str_get_str(path_in);
	char f_path[64] = "";
	int len = 0;
	
    if (path[0] == '\0') {
        path = ".";
    } else if(path[0] == '/' && path[1] == '\0') {
        path = "";
    } else {
        //do nothing
    }
	
	if(self->readonly)
	{
		sprintf(f_path, "%s%s", "/bak", path);
		iter->readonly = true;
	}
	else
	{
		sprintf(f_path, "%s%s", "/usr", path);
		iter->readonly = false;
	}
	len = (strlen(f_path) < 60) ? strlen(f_path) : 60;
	memset (iter->ilist_path, 0, sizeof(iter->ilist_path));
	strncpy(iter->ilist_path, f_path, len); 
	
    iter->dir = Helios_opendir(f_path);
    if (iter->dir == NULL) {
        mp_raise_OSError(-1);
    }
    return MP_OBJ_FROM_PTR(iter);
}
static MP_DEFINE_CONST_FUN_OBJ_2(vfs_quecfs_ilistdir_obj, vfs_quecfs_ilistdir);

static mp_obj_t vfs_quecfs_mkdir(mp_obj_t self_in, mp_obj_t path_in)
{
	mp_obj_vfs_quecfs_t *self = MP_OBJ_TO_PTR(self_in);
    const char *path = mp_vfs_quecfs_make_path(self, path_in);
    char f_path[64] = "";

	if(self->readonly)
	{
		sprintf(f_path, "%s%s", "/bak", path);
	}
	else
	{
		sprintf(f_path, "%s%s", "/usr", path);
	}
	
	int ret = Helios_mkdir(f_path, 0777);
	if (ret != 0) {
        mp_raise_OSError(ret);
    }
	
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(vfs_quecfs_mkdir_obj, vfs_quecfs_mkdir);

static mp_obj_t vfs_quecfs_remove(mp_obj_t self_in, mp_obj_t path_in) {
    mp_obj_vfs_quecfs_t *self = MP_OBJ_TO_PTR(self_in);
    const char *path = mp_vfs_quecfs_make_path(self, path_in);
	char f_path[64] = "";
    Helios_fstat_t file_stat = {0};

	if(self->readonly)
	{
		sprintf(f_path, "%s%s", "/bak", path);
	}
	else
	{
		sprintf(f_path, "%s%s", "/usr", path);
	}
#if defined(PLAT_SONY_ALT1350)
    int ret = Helios_fstat(f_path, &file_stat);
    if (ret == 0) 
	{
        if(file_stat.f_isdir)
        {
            ret = Helios_rmdir(f_path);
        }
        else
        {
            ret = Helios_remove(f_path);
        }
        if(ret != 0)
        {
            mp_raise_OSError(ret);
        }
	}
    else
    {
        mp_raise_OSError(ret);
    }
#else
    int ret = Helios_remove(f_path);
	if (ret != 0) 
    {
        mp_raise_OSError(ret);
    }
#endif
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(vfs_quecfs_remove_obj, vfs_quecfs_remove);

static mp_obj_t vfs_quecfs_rename(mp_obj_t self_in, mp_obj_t old_path_in, mp_obj_t new_path_in) {
    mp_obj_vfs_quecfs_t *self = MP_OBJ_TO_PTR(self_in);
    const char *old_path = mp_vfs_quecfs_make_path(self, old_path_in);
    const char *new_path = mp_obj_str_get_str(new_path_in);
    vstr_t path_new;
    vstr_init(&path_new, vstr_len(&self->cur_dir));
    if (new_path[0] != '/') {
        vstr_add_strn(&path_new, vstr_str(&self->cur_dir), vstr_len(&self->cur_dir));
    }
    vstr_add_str(&path_new, new_path);
	char o_path[64] = "";
	char n_path[64] = "";

	if(self->readonly)
	{
		sprintf(o_path, "%s%s", "/bak", old_path);
		sprintf(n_path, "%s%s", "/bak", vstr_null_terminated_str(&path_new));
	}
	else
	{
		sprintf(o_path, "%s%s", "/usr", old_path);
		sprintf(n_path, "%s%s", "/usr", vstr_null_terminated_str(&path_new));
	}

	
    int ret = Helios_rename(o_path, n_path);
    vstr_clear(&path_new);
    if (ret != 0) {
        mp_raise_OSError(ret);
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(vfs_quecfs_rename_obj, vfs_quecfs_rename);

static mp_obj_t vfs_quecfs_rmdir(mp_obj_t self_in, mp_obj_t path_in) {
    mp_obj_vfs_quecfs_t *self = MP_OBJ_TO_PTR(self_in);
    const char *path = mp_vfs_quecfs_make_path(self, path_in);
    char f_path[64] = "";

	if(self->readonly)
	{
		sprintf(f_path, "%s%s", "/bak", path);
	}
	else
	{
		sprintf(f_path, "%s%s", "/usr", path);
	}
	
	int ret = Helios_rmdir(f_path);
	if (ret != 0) {
        mp_raise_OSError(ret);
    }
	
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(vfs_quecfs_rmdir_obj, vfs_quecfs_rmdir);

static mp_obj_t vfs_quecfs_stat(mp_obj_t self_in, mp_obj_t path_in) {
    int ret;
	Helios_fstat_t file_stat = {0};
	mp_obj_vfs_quecfs_t *self = MP_OBJ_TO_PTR(self_in);
    const char *path = mp_vfs_quecfs_make_path(self, path_in);
	char f_path[64] = "";
	
	if(self->readonly)
	{
		sprintf(f_path, "%s%s", "/bak", path);
	}
	else
	{
		sprintf(f_path, "%s%s", "/usr", path);
	}
	
	ret = Helios_fstat(f_path, &file_stat);
	if (ret != 0) {
        mp_raise_OSError(ret);
    }
	
    mp_obj_tuple_t *t = MP_OBJ_TO_PTR(mp_obj_new_tuple(10, NULL));
    t->items[0] = MP_OBJ_NEW_SMALL_INT(file_stat.f_isdir ? MP_S_IFDIR : MP_S_IFREG);// st_mode
    t->items[1] = MP_OBJ_NEW_SMALL_INT(0);//mp_obj_new_int_from_uint(sb.st_ino);// st_ino
    t->items[2] = MP_OBJ_NEW_SMALL_INT(0);//mp_obj_new_int_from_uint(sb.st_dev);// st_dev
    t->items[3] = MP_OBJ_NEW_SMALL_INT(0);//mp_obj_new_int_from_uint(sb.st_nlink);// st_nlink
    t->items[4] = MP_OBJ_NEW_SMALL_INT(0);//mp_obj_new_int_from_uint(sb.st_uid);// st_uid
    t->items[5] = MP_OBJ_NEW_SMALL_INT(0);//mp_obj_new_int_from_uint(sb.st_gid);// st_gid
    t->items[6] = mp_obj_new_int_from_uint(file_stat.f_size);// st_size
    t->items[7] = MP_OBJ_NEW_SMALL_INT(0);//mp_obj_new_int_from_uint(sb.st_atime);// st_atime
    t->items[8] = MP_OBJ_NEW_SMALL_INT(0);//mp_obj_new_int_from_uint(sb.st_mtime);// st_mtime
    t->items[9] = MP_OBJ_NEW_SMALL_INT(0);//mp_obj_new_int_from_uint(sb.st_ctime);// st_ctime
    return MP_OBJ_FROM_PTR(t);
}
static MP_DEFINE_CONST_FUN_OBJ_2(vfs_quecfs_stat_obj, vfs_quecfs_stat);


static mp_obj_t vfs_quecfs_statvfs(mp_obj_t self_in, mp_obj_t path_in) {
	mp_obj_vfs_quecfs_t *self = MP_OBJ_TO_PTR(self_in);
	int ret;
	Helios_fs_info_t fs_info = {0};
    // const char *path = mp_obj_str_get_str(path_in);
	//char f_path[64] = "";
    char fs_disk = '\0';
	if(self->readonly)
	{
		fs_disk = 'B';
	}
	else
	{
		fs_disk = 'U';
	}

	ret = helios_fs_info_get(fs_disk, &fs_info);
	if (ret != 0) {
        mp_raise_OSError(ret);
    }
	
    mp_obj_tuple_t *t = MP_OBJ_TO_PTR(mp_obj_new_tuple(10, NULL));
    t->items[0] = MP_OBJ_NEW_SMALL_INT(fs_info.f_bsize);// f_bsize
    t->items[1] = t->items[0];// f_frsize
    t->items[2] = MP_OBJ_NEW_SMALL_INT(fs_info.f_blocks);//MP_OBJ_NEW_SMALL_INT(256);// f_blocks
    t->items[3] = MP_OBJ_NEW_SMALL_INT(fs_info.f_bfree);// f_bfree
    t->items[4] = t->items[3];// f_bavail
    t->items[5] = MP_OBJ_NEW_SMALL_INT(0); // f_files
    t->items[6] = MP_OBJ_NEW_SMALL_INT(0); // f_ffree
    t->items[7] = MP_OBJ_NEW_SMALL_INT(0); // f_favail
    t->items[8] = MP_OBJ_NEW_SMALL_INT(0); // f_flags
    t->items[9] = MP_OBJ_NEW_SMALL_INT(64);
    return MP_OBJ_FROM_PTR(t);
}
static MP_DEFINE_CONST_FUN_OBJ_2(vfs_quecfs_statvfs_obj, vfs_quecfs_statvfs);

static const mp_rom_map_elem_t vfs_quecfs_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_mount), MP_ROM_PTR(&vfs_quecfs_mount_obj) },
    { MP_ROM_QSTR(MP_QSTR_umount), MP_ROM_PTR(&vfs_quecfs_umount_obj) },
    { MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&vfs_quecfs_open_obj) },

    { MP_ROM_QSTR(MP_QSTR_chdir), MP_ROM_PTR(&vfs_quecfs_chdir_obj) },
    { MP_ROM_QSTR(MP_QSTR_getcwd), MP_ROM_PTR(&vfs_quecfs_getcwd_obj) },
    { MP_ROM_QSTR(MP_QSTR_ilistdir), MP_ROM_PTR(&vfs_quecfs_ilistdir_obj) },
    { MP_ROM_QSTR(MP_QSTR_mkdir), MP_ROM_PTR(&vfs_quecfs_mkdir_obj) },
    { MP_ROM_QSTR(MP_QSTR_remove), MP_ROM_PTR(&vfs_quecfs_remove_obj) },
    { MP_ROM_QSTR(MP_QSTR_rename), MP_ROM_PTR(&vfs_quecfs_rename_obj) },
    { MP_ROM_QSTR(MP_QSTR_rmdir), MP_ROM_PTR(&vfs_quecfs_rmdir_obj) },
    { MP_ROM_QSTR(MP_QSTR_stat), MP_ROM_PTR(&vfs_quecfs_stat_obj) },
    { MP_ROM_QSTR(MP_QSTR_statvfs), MP_ROM_PTR(&vfs_quecfs_statvfs_obj) },
};
static MP_DEFINE_CONST_DICT(vfs_quecfs_locals_dict, vfs_quecfs_locals_dict_table);

static const mp_vfs_proto_t vfs_quecfs_proto = {
    .import_stat = mp_vfs_quecfs_import_stat,
};

MP_DEFINE_CONST_OBJ_TYPE
(
    mp_type_vfs_quecfs,
    MP_QSTR_VfsQuecfs,
    MP_TYPE_FLAG_NONE,
    make_new, vfs_quecfs_make_new,
    protocol, &vfs_quecfs_proto,
    locals_dict, &vfs_quecfs_locals_dict
);

#endif //MICROPY_VFS_QUECFS