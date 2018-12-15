cmake_minimum_required(VERSION 3.12)
set(TARGET_NAME qtdups)

project(${TARGET_NAME})

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall --std=c++17 -g -pthread)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)

# Source files
set(SOURCES
        application/main.cpp
        ui/mainwindow.cpp
        finder/QtDupsFinder.cpp
        finder/QtDupsFinder.h
        ui/dialogwindow.h
        )

# User interface files
set(FORMS
        forms/mainwindow.ui
        forms/dialogwindow.ui
        forms/errorswindow.ui
        )


# Shared libraries
set(LIBRARIES
        Qt5::Widgets
        pthread
        )

# Generate additional sources with MOC and UIC
qt5_wrap_ui(UIC_SOURCES ${FORMS})

# Set target
add_executable(${TARGET_NAME} ${SOURCES} ${HEADERS} ${UIC_SOURCES})

# Link with libraries
target_link_libraries(${TARGET_NAME} ${LIBRARIES})