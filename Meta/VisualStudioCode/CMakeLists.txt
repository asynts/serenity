cmake_minimum_required(VERSION 3.18)
project(serenity)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

set(SERENITY_KERNEL "off" CACHE BOOL "If the sources are build in kernel or userland mode.")

file(GLOB_RECURSE AK_SOURCES "../../AK/*.cpp")
file(GLOB_RECURSE APPLICATIONS_SOURCES "../../Applications/*.cpp")
file(GLOB_RECURSE DEMOS_SOURCES "../../Demos/*.cpp")
file(GLOB_RECURSE DEVTOOLS_SOURCES "../../DevTools/*.cpp")
file(GLOB_RECURSE GAMES_SOURCES "../../Games/*.cpp")
file(GLOB_RECURSE KERNEL_SOURCES "../../Kernel/*.cpp")
file(GLOB_RECURSE LIBRARIES_SOURCES "../../Libraries/*.cpp")
file(GLOB_RECURSE MENU_APPLETS_SOURCES "../../MenuApplets/*.cpp")
file(GLOB_RECURSE PORTS_SOURCES "../../Ports/*.cpp")
file(GLOB_RECURSE SERVERS_SOURCES "../../Services/*.cpp")
file(GLOB_RECURSE SHELL_SOURCES "../../Shell/*.cpp")
file(GLOB_RECURSE TESTS_SOURCES "../../Tests/*.cpp")
file(GLOB_RECURSE USERLAND_SOURCES "../../Userland/*.cpp")

set(INCLUDE_DIRS
    "../.."
    "../../Kernel"
    "../../Libraries"
    "../../Libraries/LibC"
    "../../Libraries/LibPthread"
    "../../Services"
    "../../Toolchain/Local/i686-pc-serenity/include/c++/10.1.0")

add_library(serenity
    ${AK_SOURCES}
    ${APPLICATIONS_SOURCES}
    ${DEMOS_SOURCES}
    ${DEVTOOLS_SOURCES}
    ${GAMES_SOURCES}
    ${KERNEL_SOURCES}
    ${LIBRARIES_SOURCES}
    ${MENU_APPLETS_SOURCES}
    ${PORTS_SOURCES}
    ${SERVERS_SOURCES}
    ${SHELL_SOURCES}
    ${TESTS_SOURCES}
    ${USERLAND_SOURCES})

if(${SERENITY_KERNEL})
    target_compile_definitions(serenity PRIVATE KERNEL)
endif()

target_compile_definitions(serenity PRIVATE __serenity__ SANITIZE_PTRS DEBUG)
target_include_directories(serenity PRIVATE ${INCLUDE_DIRS})
