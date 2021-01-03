compile_gml(WriterWindow.gml WriterWindowGML.h writer_window_gml)

file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp Model/*.cpp)
set(SOURCES ${SOURCES} WriterWindowGML.h)

serenity_app(Writer ICON app-writer)
target_link_libraries(Writer LibWeb)
