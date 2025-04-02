# Copyright (c) Quectel Wireless Solution, Co., Ltd.All Rights Reserved.
#  
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#  
#     http://www.apache.org/licenses/LICENSE-2.0
#  
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#!python3
# -*- coding:utf-8 -*-

import uos
import ujson
import checksum
import file_crc32
import ql_fs

backup_root_dir = '/bak'
backup_restore_flag_file = '/bak/backup_restore.json'
backup_restore_flag_file_max_size = 64


class _checkError(Exception):
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return repr(self.value)


def _check():
    if not ql_fs.path_exists(backup_restore_flag_file):
        raise _checkError('%s not exist' % backup_restore_flag_file)
    else:
        fp = open(backup_restore_flag_file, 'rt')
        fp.seek(0)
        fr = fp.read(backup_restore_flag_file_max_size)
        fp.close()
        if not fr:
            raise _checkError('%s is empty' % backup_restore_flag_file)
        return fr


def _get_backup_restore_flag():
    json_str = _check()
    backup_restore_flag = ujson.loads(json_str)
    return backup_restore_flag['enable']


def bak_update(file, data):
    _backup_root_dir = '/_bak'
    if not file.startswith("/"):
        file = "/" + file
    if not isinstance(data, dict) or ql_fs.path_exists(file):
        return -1
    try:
        backup_restore_flag = _get_backup_restore_flag()
    except Exception as e:
        return -2
    if backup_restore_flag:
        _fs = uos.VfsLfs1(32, 32, 32, "customer_backup_fs")
        uos.mount(_fs, _backup_root_dir)
        res = 0
        try:
            back_file = _backup_root_dir + file
            res = ql_fs.touch(back_file, data)
            if not ql_fs.path_exists(file):
                res = ql_fs.touch(file, dict())
            ql_fs.file_copy(file, back_file)
            code = checksum.bak_update(file_name=back_file)
            if code is not None:
                checksum.usr_update(file_name=file)
            else:
                raise Exception("checksum failed")
        except Exception as e:
            print(e)
            uos.remove(file)
            res = -5
        uos.umount(_backup_root_dir)
        return res
    else:
        return -3


def bak_file(file):
    _backup_root_dir = '/_bak'
    if not file.startswith("/"):
        file = "/" + file
    if not ql_fs.path_exists(file):
        return -1
    try:
        backup_restore_flag = _get_backup_restore_flag()
    except Exception as e:
        return -2
    if backup_restore_flag:
        _fs = uos.VfsLfs1(32, 32, 32, "customer_backup_fs")
        uos.mount(_fs, _backup_root_dir)
        res = 1
        try:
            back_file = _backup_root_dir + file
            status = ql_fs.file_copy(back_file, file)
            if status:
                code = checksum.bak_update(file_name=back_file)
                if code is not None:
                    checksum.usr_update(file_name=file)
                else:
                    res = -4
            else:
                res = -6
        except Exception as e:
            res = -5
        uos.umount(_backup_root_dir)
        return res
    else:
        return -3        

def main():
    try:
        update_file_list = []
        backup_restore_flag = _get_backup_restore_flag()
        if backup_restore_flag != 0:
            json_str = checksum.check()
            csum = ujson.loads(json_str)
            for item in csum:
                crc32_value = file_crc32.calc(item['name'])
                if crc32_value == None or (
                        int(crc32_value, 16) != int(item['crc32'], 16)):  # crc32 mismatched, should restore backup file
                    print('- restoring', item['name'])
                    backup_file_name = backup_root_dir + '/' + item['name']
                    if not ql_fs.path_exists(backup_file_name):
                        print('~ backup file does not exist:', item['name'])
                        continue
                    ql_fs.mkdirs(ql_fs.path_dirname(item['name']))
                    ql_fs.file_copy(item['name'], backup_file_name)
                    update_file_list.append(item['name'])
            if len(update_file_list):
                print('- updating checksum')
                checksum.bulk_update(update_file_list)
                print('- done')
    except Exception as e:
        print('x backup restore procedure error:', e)
        pass
