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

'''
@Author: Pawn
@Date: 2020-12-26
@LastEditTime: 2020-12-26 14:46:08
@FilePath: systeam.py
'''
import uos
import misc
import ujson

#import uhashlib
#import ubinascii

#
def _update_jsonfile(replflag, replpswd, datacallflag):
    if "system_config.json" in uos.listdir("/usr/"):
        with open("/usr/system_config.json", "w+", encoding='utf-8') as fd:
            if replpswd:
                if datacallflag != -1:
                    json_data = ujson.dumps({"replFlag": replflag, "replPswd": replpswd, "datacallFlag": datacallflag})
                    fd.write(json_data)
                else:
                    json_data = ujson.dumps({"replFlag": replflag, "replPswd": replpswd})
                    fd.write(json_data)
            else:
                if datacallflag != -1:
                    json_data = ujson.dumps({"replFlag": replflag, "datacallFlag": datacallflag})
                    fd.write(json_data)
                else:
                    json_data = ujson.dumps({"replFlag": replflag})
                    fd.write(json_data)
                    
#key:'password'
def replSetEnable(flag=0, **kw_args):
    pswd = kw_args.get("password", None)
    try:
        import uhashlib
        import ubinascii
    except:
        if pswd:
            print('The parameter [Password] is not supported')
            return -1;
        if flag != 0 and flag != 1:
            print('flag is invaild')
            return -1
        if "system_config.json" in uos.listdir("/usr/"):
            datacall_flag = -1
            try:
                with open("/usr/system_config.json", "r", encoding='utf-8') as fd:
                    json_data = ujson.load(fd)
                    datacall_flag = json_data.get("datacallFlag",-1)
                    
                with open("/usr/system_config.json", "w+", encoding='utf-8') as fd:
                    #ujson.load()
                    if datacall_flag != -1:
                        json_data = ujson.dumps({"replFlag": flag, "datacallFlag": datacall_flag})
                        fd.write(json_data)
                    else:
                        repl_data = ujson.dumps({"replFlag": flag})
                        fd.write(repl_data)
            except:
                raise OSError("The system_config.JSON file is abnormal, please check!")
        misc.replEnable(flag)
        return 0
        
    #pswd = kw_args.get("password", None)

    if flag == 2:
        if pswd:
            return -1;
        else:
            return misc.replEnable(flag)
            
    if pswd:
        #password length:[6~12]
        if len(pswd) > 12 or len(pswd) < 6:
            print("Password length must be [6~12]!")
            return -1

        #password must be accii
        for i in range(len(pswd)):
            if pswd[i] > '\x7e' or pswd[i] < '\x21':
                print("The password contains invalid characters!")
                return -1
        
        hash_obj = uhashlib.sha256()
        hash_obj.update(pswd)
        res = hash_obj.digest()
        hex_msg = ubinascii.hexlify(res)
    
    if "system_config.json" in uos.listdir("/usr/"):
        datacall_flag = -1
        reset_needed_flag = 1
        try:
            with open("/usr/system_config.json", "r", encoding='utf-8') as fd:
                json_data = ujson.load(fd)
                datacall_flag = json_data.get("datacallFlag",-1)
                repl_pswd = json_data.get("replPswd",None)
                repl_flag = json_data.get("replFlag",0)
                
            if pswd:
                if flag:
                    if repl_flag == 0:
                        misc.replUpdatePassswd(hex_msg)
                        misc.replEnable(flag, hex_msg)
                        _update_jsonfile(flag, hex_msg, datacall_flag)
                    else:
                        if repl_pswd:
                            if repl_pswd != hex_msg.decode():
                                print('Incorrect password!')
                                return -1
                            else:
                                misc.replUpdatePassswd(repl_pswd, repl_pswd)
                                misc.replEnable(flag, repl_pswd)
                                #_update_jsonfile(flag, repl_pswd, datacall_flag)
                        else:
                            print("illegal operation")
                            return -1
                else:#flag=0
                    if repl_flag == 0:
                        misc.replEnable(flag)
                    else:
                        if repl_pswd:
                            if repl_pswd != hex_msg.decode():
                                print('Incorrect password!')
                                return -1
                            else:   
                                misc.replEnable(flag, repl_pswd)
                                misc.replUpdatePassswd('', repl_pswd)
                                _update_jsonfile(flag, '', datacall_flag)
                        else:
                            print("illegal operation")
                            return -1
            #!pswd
            else:
                _update_jsonfile(flag, '', datacall_flag)
                return misc.replEnable(flag)
        except:
            raise OSError("The system_config.JSON file is abnormal, please check!")
    return 0

def replChangPswd(**kw_args):
    try:
        import uhashlib
        import ubinascii
    except:
        raise OSError("DO NOT SUPPORT!")
        
    old_pswd = kw_args.get("old_password", None)
    new_pswd = kw_args.get("new_password", None)

    if old_pswd and new_pswd:
        #password length:[6~12]
        if len(old_pswd) > 12 or len(old_pswd) < 6:
            print("Password length must be [6~12]!")
            return -1
        if len(new_pswd) > 12 or len(new_pswd) < 6:
            print("Password length must be [6~12]!")
            return -1

        #password must be accii
        for i in range(len(old_pswd)):
            if old_pswd[i] > '\x7e' or old_pswd[i] < '\x21':
                print("The password contains invalid characters!")
                return -1
        for i in range(len(new_pswd)):
            if new_pswd[i] > '\x7e' or new_pswd[i] < '\x21':
                print("The password contains invalid characters!")
                return -1
    else:
        print("illegal operation")
        return -1
            
    if "system_config.json" in uos.listdir("/usr/"):
        datacall_flag = -1
        try:
            with open("/usr/system_config.json", "r", encoding='utf-8') as fd:
                json_data = ujson.load(fd)
                datacall_flag = json_data.get("datacallFlag",-1)
                repl_pswd = json_data.get("replPswd",None)
                repl_flag = json_data.get("replFlag",0)
                
            if old_pswd and new_pswd:
                hash_obj = uhashlib.sha256()
                hash_obj.update(old_pswd)
                res = hash_obj.digest()
                hex_msg = ubinascii.hexlify(res)

                if repl_pswd:
                    if repl_pswd == hex_msg.decode():
                        hash_obj1 = uhashlib.sha256()
                        hash_obj1.update(new_pswd)
                        res1 = hash_obj1.digest()
                        hex_msg1 = ubinascii.hexlify(res1)
                        misc.replUpdatePassswd(hex_msg1, repl_pswd)
                        _update_jsonfile(repl_flag, hex_msg1, datacall_flag)
                    else:
                        print('Incorrect password!')
                        return -1
                else:
                    print("Please set password first!")
                    return -1
        except:
            raise OSError("The system_config.JSON file is abnormal, please check!")
            
    return 0