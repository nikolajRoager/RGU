cmake_minimum_required(VERSION 3.29)

# Project name and version
project(MyProject VERSION 1.0)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)



#NOT ACTUALLY TESTED ON VS
if (MSVC)
    # warning level 4
    add_compile_options(/W4)
else()
    # additional warnings
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -lpthread -O3)#Yep, I will NOT even compile anything which has a warning
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(NCURSES ncurses)
link_libraries(${NCURSES_LIBRARIES})

pkg_check_modules(SDL2 sdl2)
pkg_check_modules(SDL2_image SDL2_image)
pkg_check_modules(SDL2_mixer SDL2_mixer)
link_libraries(${SDL2_LIBRARIES})
link_libraries(${SDL2_image_LIBRARIES})
link_libraries(${SDL2_mixer_LIBRARIES})



# Set the output directories for libraries and binaries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include subdirectories
add_subdirectory(src)
