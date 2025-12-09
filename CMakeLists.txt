cmake_minimum_required(VERSION 3.31)
project(MWVS)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(USE_ZLIB OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        IXWebSocket
        GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
        GIT_TAG        master
)

FetchContent_MakeAvailable(IXWebSocket)

add_executable(MWVS src/main.cpp libs/json.hpp)

target_link_libraries(MWVS PRIVATE ixwebsocket)

if (MSVC)
        add_subdirectory("D:/MineWeb/MineWebServer"
                "${CMAKE_BINARY_DIR}/server-build")
else()
        add_subdirectory("/workspace/MineWebServer"
                "${CMAKE_BINARY_DIR}/server-build")
endif()

target_compile_definitions(MineWebServer PRIVATE BUILD_TYPE_DEDICATED)

target_link_libraries(MWVS PRIVATE MineWebServer)
if (MSVC)
        target_include_directories(MWVS PRIVATE "D:/MineWeb/MineWebServer")
else()
        target_include_directories(MWVS PRIVATE "/workspace/MineWebServer")
endif() 

if (MSVC)
        add_compile_options(/Od /Zi /Ob0 /RTC1 /Zo /Gm- /MP /MT)
else()
        add_compile_options(-O0 -g -Wall -Wextra -pthread)
endif()