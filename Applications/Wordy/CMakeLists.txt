file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp)

serenity_bin(Wordy)
target_link_libraries(Wordy LibGUI)
