list(APPEND SOURCE
    serseat.c
    serseat.h
    serseat.rc)

add_library(serseat SHARED ${SOURCE})
set_module_type(serseat kernelmodedriver)
target_link_libraries(serseat memcmp)
add_importlibs(serseat ntoskrnl hal)
add_cd_file(TARGET serseat DESTINATION reactos/system32/drivers NO_CAB FOR all)
