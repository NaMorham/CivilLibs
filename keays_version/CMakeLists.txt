# Sample file
cmake_minimum_required(VERSION 2.8)

project(keays_version)

set(hdrs
    include/keays_version.h
)
source_group("Headers" FILES ${hdrs})

if(WIN32)
set(rsrcs
)
source_group("Resources" FILES ${rsrcs})
endif(WIN32)

set(srcs
    src/keays_version.cpp
)
source_group("Source" FILES ${srcs})

include_directories(
    ../Leakwatcher
)

add_definitions(
    -DKEAYS_MATH_EXPORTS
)

add_library(keays_version STATIC
    ${srcs}
    ${hdrs}
    ${rsrcs}
)

target_link_libraries(keays_version
)
