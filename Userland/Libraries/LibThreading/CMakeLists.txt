file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp)

serenity_lib(LibThreading threading)
target_link_libraries(LibThreading LibC LibPthread)
