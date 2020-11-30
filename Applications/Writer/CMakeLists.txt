compile_json_gui(WriterWidget.json WriterWidgetUI.h writer_widget_ui_json)

file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp DOM/*.cpp Layout/*.cpp)
set(SOURCES ${SOURCES} WriterWidgetUI.h)

serenity_bin(Writer)
target_link_libraries(Writer LibGUI LibWeb LibGfx)
