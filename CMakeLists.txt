cmake_minimum_required(VERSION 3.31)
project(MWVS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(USE_ZLIB OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        IXWebSocket
        GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
        GIT_TAG        master
)

FetchContent_MakeAvailable(IXWebSocket)

add_executable(MWVS src/main.cpp)

target_link_libraries(MWVS PRIVATE ixwebsocket)

add_subdirectory("D:/MineWeb/MineWebServer"
        "${CMAKE_BINARY_DIR}/server-build")

target_link_libraries(MWVS PRIVATE MineWebServer)

target_include_directories(MWVS PRIVATE "D:/MineWeb/MineWebServer")

add_compile_options(/Od /Zi /Ob0 /RTC1 /Zo /Gm- /MP /MT)