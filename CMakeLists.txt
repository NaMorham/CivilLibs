# Sample file
cmake_minimum_required(VERSION 2.8)

project(CivilLibs)

add_subdirectory(keays_types)
add_subdirectory(keays_version)
add_subdirectory(keays_math)
add_dependencies(keays_math keays_types)
add_subdirectory(keays_kerb)
add_dependencies(keays_kerb keays_math keays_version)
