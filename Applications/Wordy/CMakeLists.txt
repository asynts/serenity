file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp Nodes/*.cpp)

serenity_bin(Wordy)
target_link_libraries(Wordy LibGUI)
