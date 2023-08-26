cmake_minimum_required(VERSION 3.13)

project(viewers)

add_definitions("-Werror=return-type -Werror=unused-result")

set(MAIN_PROJECT_DIR ${CMAKE_SOURCE_DIR})

set(LOGIC_SOURCES logic/ByteDocument.cpp
        logic/AbstractView.cpp
        logic/LineIndexedDocument.cpp
        logic/LineView.cpp
        logic/Wrap.cpp
        logic/IByteAccess.h
        logic/LineDeque.h
        logic/LineDeque.cpp
        logic/DString.h
        logic/DString.cpp
        logic/AbstractView.h
)

set(WID_SOURCES
        wid/TextViewer.cpp wid/TextViewer.h wid/PaintArea.cpp wid/PaintArea.h
)

add_subdirectory(examples/viewer)

add_executable(main ${LOGIC_SOURCES}
        misc/util.cpp
        main.cpp)
