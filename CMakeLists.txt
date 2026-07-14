cmake_minimum_required(VERSION 3.31)
project(MWVS)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(USE_ZLIB OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        IXWebSocket
        GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
        GIT_TAG master
)

FetchContent_MakeAvailable(IXWebSocket)

add_executable(MWVS src/main.cpp libs/json.hpp)

target_link_libraries(MWVS PRIVATE ixwebsocket)

FetchContent_Declare(
        lua
        GIT_REPOSITORY https://github.com/marovira/lua
        GIT_TAG 4737fb98d04a3e2fd365e55d8cb95e4c961db3a5
)

FetchContent_MakeAvailable(lua)

set(BUILD_TYPE_DEDICATED ON)

if (MINGW OR MSVC)
        add_subdirectory("D:/MineWeb/MineWebServer"
                "${CMAKE_BINARY_DIR}/server-build")
else()
        add_subdirectory("/workspace/MineWebServer"
                "${CMAKE_BINARY_DIR}/server-build")
endif()

if (MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /Zi /Ob0 /RTC1 /Zo /Gm- /MP /MT")
else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g -Wall -Wextra -pthread")
endif()

target_compile_definitions(MineWebServer PUBLIC BUILD_TYPE_DEDICATED)

target_link_libraries(MineWebServer PUBLIC lua::lua)

target_link_libraries(MWVS PRIVATE MineWebServer)

if (MINGW OR MSVC)
        target_include_directories(MWVS PRIVATE "D:/MineWeb/MineWebServer")
else()
        target_include_directories(MWVS PRIVATE "/workspace/MineWebServer")
endif()