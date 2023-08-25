cmake_minimum_required(VERSION 3.26)
project(viewer)

add_definitions("-Werror=return-type -Werror=unused-result")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)

set(LOGIC_SOURCES logic/AbstractDocument.cpp
        logic/AbstractView.cpp
        logic/LineDocument.cpp
        logic/LineView.cpp
        logic/Wrap.cpp
        logic/IByteAccess.h
        logic/LineDeque.h
        logic/LineDeque.cpp
        logic/DString.h
        logic/DString.cpp
)

add_executable(${PROJECT_NAME} ${LOGIC_SOURCES}
        misc/util.cpp
        main.cpp)
