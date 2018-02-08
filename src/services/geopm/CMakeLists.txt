set(CALIPER_GEOPM_SOURCES
    Geopm.cpp)

include_directories("/g/g92/marathe1/quartz/local/include/") 
add_service_sources(${CALIPER_GEOPM_SOURCES})
add_caliper_service("geopm")
#    shim_config.c
#    shim_power.c

