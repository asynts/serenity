file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp)

serenity_lib(LibChrono chrono)
target_link_libraries(LibChrono LibC)
