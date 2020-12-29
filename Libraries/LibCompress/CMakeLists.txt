file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp)

serenity_lib(LibCompress compression)
target_link_libraries(LibCompress LibC LibCrypto)
