file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp)

serenity_bin(Writer)
target_link_libraries(Writer LibWeb)
