import uos

try:
    ufs = uos.VfsQuecfs("customer_fs")
    uos.mount(ufs, '/usr')
    print("File system mounted successfully")
    if ufs != None:
        try:
            import app_fota
            fota = app_fota.new() # app fota update main process
            fota.update()
        except Exception as e: 
            print("update error")
except Exception as e:
    print("File system mount error:" + str(e))