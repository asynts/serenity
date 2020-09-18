add_library(LibDummy STATIC Dummy.cpp)
target_link_libraries(LibDummy LibC)
target_compile_options(LibDummy PRIVATE -Os)
