cmake_minimum_required(VERSION 3.13)

project(viewers)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_definitions("-Werror=return-type -Werror=unused-result")

set(MAIN_PROJECT_DIR ${CMAKE_SOURCE_DIR})

find_package(ICU REQUIRED COMPONENTS uc)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PCRE2 REQUIRED IMPORTED_TARGET GLOBAL libpcre2-8)
find_package(qcodeedit REQUIRED)
find_package(qcodeedit-kate REQUIRED)

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
        logic/Searcher.cpp
        logic/Searcher.h
        logic/RegexSearcher.cpp
        logic/RegexSearcher.h
)

set(MISC_SOURCES
        misc/selection.cpp
)

set(WID_SOURCES
        wid/TextViewer.cpp wid/TextViewer.h wid/PaintArea.cpp wid/PaintArea.h
        wid/FindBar.cpp wid/FindBar.h
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
        test/testProportional.cpp
        test/testProportionalL.cpp
        test/testSearcher.cpp
        test/testFold.cpp
)

target_link_libraries(gtest_viewer gtest gtest_main ICU::uc PkgConfig::PCRE2 qcodeedit::qcodeedit)

target_include_directories(gtest_viewer PRIVATE ${CMAKE_SOURCE_DIR})