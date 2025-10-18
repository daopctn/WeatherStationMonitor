cmake_minimum_required(VERSION 3.16)

project(WeatherStationMonitor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network Sql)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/WeatherWorker.cpp
    src/PythonBridge.cpp
    src/DatabaseManager.cpp
    src/DatabaseThread.cpp
    src/ThreadManager.cpp
    src/Spinner.cpp
)

set(HEADERS
    include/MainWindow.h
    include/WeatherWorker.h
    include/WeatherData.h
    include/PythonBridge.h
    include/DatabaseManager.h
    include/DatabaseThread.h
    include/ThreadManager.h
    include/Spinner.h
)

set(UI_FILES
    ui/MainWindow.ui
)

set(RESOURCES
    resources/icons/data/data.qrc
    resources/icons/weather/weather_icons.qrc
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${RESOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${Python3_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
    Qt5::Sql
    Python3::Python
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)