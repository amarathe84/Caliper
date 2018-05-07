set(CALIPER_GEOPM_SOURCES
    Geopm.cpp
    platform.cpp
    shim_config.cpp
    cpuid.c
    shift.c
    debug.cpp
    task.cpp
    )

include_directories("/g/g92/marathe1/catalyst/local/include") 
include_directories("/g/g92/marathe1/install-ecp/include/") 
include_directories("/g/g92/marathe1/work/ecp/software/catalyst/Caliper/include/caliper") 
list(APPEND CALIPER_EXTERNAL_LIBS " -DDEBUGLEVEL=2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUGLEVEL=2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUGLEVEL=2")

add_service_sources(${CALIPER_GEOPM_SOURCES})
add_caliper_service("geopm")

