compile_gml(WriterWidget.gml WriterWidgetGML.h writer_widget_gml)

file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp)
set(SOURCES ${SOURCES} WriterWidgetGML.h)

serenity_app(Writer ICON app-writer)
target_link_libraries(Writer LibWeb)
