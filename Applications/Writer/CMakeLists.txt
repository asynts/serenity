compile_json_gui(MainWindow.json MainWindowUI.h main_window_ui_json)
compile_json_gui(InspectorWindow.json InspectorWindowUI.h inspector_window_ui_json)

file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp)
set(SOURCES ${SOURCES} MainWindowUI.h InspectorWindowUI.h)

serenity_bin(Writer)
target_link_libraries(Writer LibGUI LibWeb)
