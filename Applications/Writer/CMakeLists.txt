file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp Model/*.cpp)

serenity_bin(Writer)
target_link_libraries(Writer LibWeb)
