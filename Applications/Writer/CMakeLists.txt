file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp DOM/*.cpp)

serenity_bin(Writer)
target_link_libraries(Writer LibGUI LibWeb LibGfx)
