compile_json_gui(MainWindow.json MainWindowUI.h main_window_ui_json)

file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp)
set(SOURCES ${SOURCES} MainWindowUI.h)

serenity_bin(Writer)
target_link_libraries(Writer LibGUI LibWeb)
