cmake_minimum_required(VERSION 3.13)

project(version-controll)

file(GLOB_RECURSE SOURCE_FILES src/*.c src/*.h)

set(CMAKE_C_STANDARD 11)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

#Builds an executable named app using all source files in SOURCE_FILES.
add_executable(app ${SOURCE_FILES})

target_link_libraries(app PRIVATE OpenSSL::Crypto)
target_link_libraries(app PRIVATE ZLIB::ZLIB)


