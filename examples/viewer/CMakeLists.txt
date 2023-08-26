set(EXAMPLE textViewer)

cmake_minimum_required(VERSION 3.13)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

foreach(source_file ${LOGIC_SOURCES})
    list(APPEND LOGIC_SOURCES_RELATIVE ../../${source_file})
endforeach()

foreach(source_file ${WID_SOURCES})
    list(APPEND WID_SOURCES_RELATIVE ../../${source_file})
endforeach()

set(PROJECT_SOURCES
        main.cpp
        MainWindow.cpp MainWindow.h
        ${LOGIC_SOURCES_RELATIVE}
        ${WID_SOURCES_RELATIVE}
)

qt_add_executable(${EXAMPLE}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
)

target_link_libraries(${EXAMPLE} PRIVATE Qt6::Widgets)

target_include_directories(${EXAMPLE} PRIVATE ../../)

qt_finalize_executable(${EXAMPLE})
