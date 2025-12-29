# links Qt6::Widgets and Qt6::Network
cmake_minimum_required(VERSION 3.20)

# Enable Qt's automatic processing for moc/uic/rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Auto-detect Qt6 installation on Windows
if(WIN32 AND NOT DEFINED CMAKE_PREFIX_PATH)
    # Common Qt6 installation paths on Windows
    set(QT6_POSSIBLE_PATHS
        "C:/Qt/6.10.1/msvc2019_64"
        "C:/Qt/6.8.1/msvc2019_64"
        "C:/Qt/6.7.3/msvc2019_64"
        "C:/Qt/6.6.3/msvc2019_64"
        "$ENV{USERPROFILE}/Qt/6.10.1/msvc2019_64"
        "$ENV{USERPROFILE}/Qt/6.8.1/msvc2019_64"
    )
    
    foreach(QT_PATH ${QT6_POSSIBLE_PATHS})
        if(EXISTS "${QT_PATH}")
            set(CMAKE_PREFIX_PATH "${QT_PATH}" CACHE PATH "Qt6 installation path")
            message(STATUS "Auto-detected Qt6 at: ${QT_PATH}")
            break()
        endif()
    endforeach()
    
    if(NOT DEFINED CMAKE_PREFIX_PATH)
        message(WARNING "Qt6 not found in standard locations. Set CMAKE_PREFIX_PATH manually or use -DBUILD_GUI=OFF")
    endif()
endif()

# Enforce minimum Qt version to avoid tool mismatch
find_package(Qt6 6.6 COMPONENTS Widgets Network)

add_executable(gui_app
    src/main_gui.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/RestClient.cpp
    src/RestClient.h
)

target_link_libraries(gui_app
    PRIVATE Qt6::Widgets Qt6::Network
)
