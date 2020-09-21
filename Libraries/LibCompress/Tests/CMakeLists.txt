cmake_minimum_required (VERSION 3.0)
project (Lagom)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -Werror -std=c++20 -fPIC -g")

file(GLOB LIBCRYPTO_SOURCES "../../LibCrypto/*/*.cpp")

file(GLOB LIBCOMPRESS_SOURCES "../*.cpp")
file(GLOB LIBCOMPRESS_TESTS "*.cpp")

add_library(LagomCrypto ${LIBCRYPTO_SOURCES})
target_link_libraries(LagomCrypto LagomCore)

add_library(LagomCompress ${LIBCOMPRESS_SOURCES})
target_link_libraries(LagomCompress LagomCore LagomCrypto)

foreach(source ${LIBCOMPRESS_TESTS})
    get_filename_component(name ${source} NAME_WE)
    add_executable(${name} ${source})
    target_link_libraries(${name} LagomCore LagomCompress)
    add_test(
        NAME ${name}
        COMMAND ${name}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    set_tests_properties(
        ${name}
        PROPERTIES
            FAIL_REGULAR_EXPRESSION "FAIL"
    )
endforeach()
