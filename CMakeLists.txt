cmake_minimum_required(VERSION 3.16)

project(WeatherStationMonitor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)

set(SOURCES
    src/main.cpp
)

set(UI_FILES
    ui/MainWindow.ui
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${UI_FILES}
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)