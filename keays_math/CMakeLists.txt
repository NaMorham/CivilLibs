# Sample file
cmake_minimum_required(VERSION 2.8)

project(keays_math)

set(hdrs
    include/mathhelp.h
    include/keays_math.h
    include/geometry.h
)
source_group("Headers" FILES ${hdrs})

if(WIN32)
set(rsrcs
    include/resource.h
    include/keays_math.rc
)
source_group("Resources" FILES ${rsrcs})
endif(WIN32)

set(srcs
    src/geometry.cpp
    src/kmCube.cpp
    src/kmLine.cpp
)
source_group("Source" FILES ${srcs})

include_directories(
    ../Leakwatcher
    ../keays_types/include
#    ${IDZip_INCLUDE_DIRS}
#    ${Netcode_INCLUDE_DIRS}
#    ${BinIO_INCLUDE_DIRS}
#    ${RoamesLogger_INCLUDE_DIRS}
#    ${tinyformat_INCLUDE_DIRS}
#    ${CommandLineUtil_INCLUDE_DIRS}
#    ${RoamesLogger_INCLUDE_DIRS}
#    ${MathsLibrary_INCLUDE_DIRS}
#    ${Boost_IOSTREAMS_LIBRARY}
#    ${zlib_INCLUDE_DIRS}
)

add_definitions(
    -DKEAYS_MATH_EXPORTS
)

add_library(keays_math STATIC
    ${srcs}
    ${hdrs}
    ${rsrcs}
)

target_link_libraries(keays_math
#    Netcode
#    IDZip
#    ${Boost_IOSTREAMS_LIBRARY}
#    ${Boost_FILESYSTEM_LIBRARY}
#    RoamesLogger
#    CommandLineUtil
#    ${ZLIB_LIBRARIES}
#    ${OPENSSL_LIBRARIES}
)
