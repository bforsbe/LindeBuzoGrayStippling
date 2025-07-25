cmake_minimum_required(VERSION 3.10)

project(LBGStippling)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)

set(PROJECT_DIR ${PROJECT_SOURCE_DIR})

# add headers to project
set(HEADERS
        ${PROJECT_DIR}/src/mainwindow.h
        ${PROJECT_DIR}/src/stippleviewer.h
        ${PROJECT_DIR}/src/voronoidiagram.h
        ${PROJECT_DIR}/src/voronoicell.h
        ${PROJECT_DIR}/src/lbgstippling.h
        ${PROJECT_DIR}/src/settingswidget.h
)

# add sources to project
set(SOURCES
	${PROJECT_DIR}/main.cpp
	${PROJECT_DIR}/src/mainwindow.cpp
        ${PROJECT_DIR}/src/stippleviewer.cpp
        ${PROJECT_DIR}/src/voronoidiagram.cpp
        ${PROJECT_DIR}/src/lbgstippling.cpp
        ${PROJECT_DIR}/src/settingswidget.cpp
        ${PROJECT_DIR}/src/voronoicell.cpp
)

find_package(Qt5 5.10 COMPONENTS Core Widgets Svg PrintSupport REQUIRED)
find_package(OpenMP REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${Qt5Core_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${Qt5Svg_INCLUDE_DIRS} 
        ${Qt5PrintSupport_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} resources.qrc)

target_link_libraries(${PROJECT_NAME} 
	Qt5::Core
	Qt5::Widgets
	Qt5::Svg
	Qt5::PrintSupport
        OpenMP::OpenMP_CXX
        OpenGL::GL
)
