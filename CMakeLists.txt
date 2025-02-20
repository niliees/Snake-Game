cmake_minimum_required(VERSION 3.30)
project(Snake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows -static")
    # Resource-Datei hinzufügen
    enable_language(RC)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/resource.rc")
endif()

add_executable(Snake WIN32
        main.cpp
        ${APP_ICON_RESOURCE_WINDOWS}
)