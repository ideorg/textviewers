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
        logic/AbstractView.h
        logic/ByteView.cpp
        logic/ByteDeque.h
        logic/ByteDeque.cpp
        logic/ChangeableDocument.h
        logic/ChangeableDocument.cpp
)

set(MISC_SOURCES
        misc/selection.cpp
)

set(WID_SOURCES
        wid/TextViewer.cpp wid/TextViewer.h wid/PaintArea.cpp wid/PaintArea.h
)

add_subdirectory(examples/viewer)

add_executable(gtest_viewer ${LOGIC_SOURCES}
        misc/util.cpp
        test/testByteDocument.cpp
        test/testIByteAccess.cpp
        test/testBasics.cpp
        test/testBack.cpp
        test/testUtil.cpp
        test/testIByteAccessML.cpp
        test/testMaxline.cpp
        test/testWrap.cpp
        test/testPosition.cpp
        test/testPositionL.cpp
        test/testUnicode.cpp
        test/testTabs.cpp
        test/testStartX.cpp
        test/testScroll.cpp
        test/testScrollL.cpp
)

target_link_libraries(gtest_viewer gtest gtest_main)

target_include_directories(gtest_viewer PRIVATE ${CMAKE_SOURCE_DIR})