
import request
import fota

url = fota.fota_get_url()
obj = fota.fota_get_obj()
r = request.get(url)
file_size = int(r.headers['Content-Length'])

if r.status_code == 200 or r.status_code == 206:
    content = r.content
    try:
        while 1:
            c = next(content)
            length = len(c)
            print(length)
            obj.write(c, 1024)

    except StopIteration:
        r.close()

res_flush = obj.flush()
print("res_flush : {}".format(res_flush))
if res_flush != 0:
    print("flush error")

res_check = obj.verify()
print("res_check : {}".format(res_check))
if res_check != 0:
    print("verify error")