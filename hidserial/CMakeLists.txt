list(APPEND SOURCE
    hidserial.c
    hidserial.rc)

add_library(hidserial SHARED ${SOURCE})
set_module_type(hidserial kernelmodedriver)
add_importlibs(hidserial hidclass ntoskrnl hal)
add_cd_file(TARGET hidserial DESTINATION reactos/system32/drivers NO_CAB FOR all)
