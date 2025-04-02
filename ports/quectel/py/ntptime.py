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

try:
    import usocket as socket
except:
    import socket
try:
    import ustruct as struct
except:
    import struct

# (date(2000, 1, 1) - date(1900, 1, 1)).days * 24*60*60
NTP_DELTA = 3155673600

# The NTP host can be configured at runtime by doing: ntptime.host = 'myhost.org'
host = "ntp.aliyun.com"
r_host = ["pool.ntp.org", "asia.pool.ntp.org", "cn.ntp.org.cn", "cn.pool.ntp.org"]

def sethost(ntp_host=None):
    global host
    if ntp_host != None:
        host = ntp_host
        return 0
    else:
        return -1
        

def time():
    NTP_QUERY = bytearray(48)
    NTP_QUERY[0] = 0x1B
    global host
    val = None
    
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    for host_m in r_host:
        try:
            addr = socket.getaddrinfo(host, 123)[0][-1]
            s.settimeout(10)
            res = s.sendto(NTP_QUERY, addr)
            msg = s.recv(48)
            val = struct.unpack("!I", msg[40:44])[0]
            break
        except:
            host = host_m
            continue
            
    s.close()
    if val == None:
        raise OSError("Server connection failed!")
    return val - NTP_DELTA


# There's currently no timezone support in MicroPython, so
# utime.localtime() will return UTC time (as if it was .gmtime())
def settime(timezone=0):
    import machine
    import utime
    if timezone < -12 or timezone > 12:
        return -1
    t = time()
    try:
        t = t + 946656000 + timezone * 3600
        tm = utime.localtime(t)
        machine.RTC().datetime((tm[0], tm[1], tm[2], tm[6], tm[3], tm[4], tm[5], 0))
    except:
        return -1
    return 0
