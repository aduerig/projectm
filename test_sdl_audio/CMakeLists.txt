# rm CMakeCache.txt; cmake -DCMAKE_BUILD_TYPE=Release && cmake --build . -- -j && ./test_sdl2

cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

include(CheckSymbolExists)

project(test_sdl2 LANGUAGES C CXX)



# set(PROJECTM_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
# set(PROJECTM_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

set(PROJECTM_BIN_DIR "." CACHE STRING "Executable installation directory, relative to the install prefix.")
set(PROJECTM_LIB_DIR "." CACHE STRING "Library installation directory, relative to the install prefix.")
set(PROJECTM_INCLUDE_DIR "." CACHE STRING "Header installation directory, relative to the install prefix.")


add_executable(test_sdl2
        test_sdl.cpp
        )

target_link_libraries(test_sdl2
        PRIVATE
        glfw
        SDL2::SDL2
        ${CMAKE_DL_LIBS}
        )



execute_process(COMMAND hostname OUTPUT_VARIABLE HOSTNAME OUTPUT_STRIP_TRAILING_WHITESPACE)
if("${HOSTNAME}" STREQUAL "doorbell")
    message(STATUS "Hostname is doorbell!")
    include_directories("/home/pi/random/sdl_install/SDL/include/")
    link_directories("/home/pi/random/sdl_install/SDL/build/.libs/")
    target_link_libraries(YourTargetName SDL2)
else()
    message(STATUS "Hostname is NOT doorbell!")
    find_package(SDL2 REQUIRED)
endif()



# find_package(glfw3 REQUIRED)

# get_target_property(_SDL2_INCLUDE_DIR SDL2::SDL2 INTERFACE_INCLUDE_DIRECTORIES)
# if(_SDL2_INCLUDE_DIR MATCHES "(.+)/SDL2\$" AND _SDL2_TARGET_TYPE STREQUAL STATIC_LIBRARY)
#     # Check if SDL2::SDL2 is aliased to SDL2::SDL2-static (will be the case for static-only builds)
#     get_target_property(_SDL2_ALIASED_TARGET SDL2::SDL2 ALIASED_TARGET)
#     if(_SDL2_ALIASED_TARGET)
#         set(_sdl2_target ${_SDL2_ALIASED_TARGET})
#     else()
#         set(_sdl2_target SDL2::SDL2)
#     endif()

#     message(STATUS "SDL2 include dir contains \"SDL2\" subdir (SDL bug #4004) - fixing to \"${CMAKE_MATCH_1}\".")
#     set_target_properties(${_sdl2_target} PROPERTIES
#             INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_MATCH_1}"
#             )
# endif()

if(SDL2_VERSION AND SDL2_VERSION VERSION_LESS "2.0.5")
    message(FATAL_ERROR "SDL2 libraries were found, but have version ${SDL2_VERSION}. At least version 2.0.5 is required.")
endif()




