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

# !python3
# -*- coding:utf-8 -*-

from backup_restore import bak_update, bak_file
import uos, ujson, ql_fs


class SecureStorage(object):
    _usr = "usr"
    bak = "bak"
    _bak = '/_' + bak
    _back_config_path = "/_usr_config.json"
    _customer_backup_fs = "customer_backup_fs"

    @classmethod
    def write(cls, data):
        if not isinstance(data, dict):
            return -1
        if not ql_fs.path_exists(cls.bak + cls._back_config_path):
            _fs = uos.VfsLfs1(32, 32, 32, cls._customer_backup_fs)
            uos.mount(_fs, cls._bak)
            try:
                with open(cls._bak + cls._back_config_path, "w") as f:
                    f.write(ujson.dumps(data))
                ql_fs.file_copy(cls._usr + cls._back_config_path, cls.bak + cls._back_config_path)
            except Exception as e:
                return -2
            else:
                return 0
            finally:
                uos.umount(cls._bak)
        else:
            return -3

    @classmethod
    def read(cls):
        if not ql_fs.path_exists(cls._usr + cls._back_config_path):
            if ql_fs.path_exists(cls.bak + cls._back_config_path):
                ql_fs.file_copy(cls._usr + cls._back_config_path, cls.bak + cls._back_config_path)
            else:
                return None
        with open(cls._usr + cls._back_config_path, "r") as f:
            return ujson.load(f)

    @classmethod
    def add(cls, file, data):
        return bak_update(file, data)

    @classmethod
    def file(cls, file):
        return bak_file(file)


JsonFile = SecureStorage
