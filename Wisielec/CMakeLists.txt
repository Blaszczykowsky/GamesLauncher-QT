cmake_minimum_required(VERSION 3.16)
project(MultiGameLauncher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

add_executable(MultiGameLauncher
    main.cpp
    launcher.h
    launcher.cpp
    game_config.h

    # Wisielec
    wisielec_window.h
    wisielec_window.cpp
    game_logic.h
    game_logic.cpp

    # Kosci
    kosci_window.h
    kosci_window.cpp
    kosci_logic.h
    kosci_logic.cpp
    kosci_network.h
    kosci_network.cpp
    kosci_config.h
    oknogry.ui

    # Zasoby
    zasoby.qrc
)

target_link_libraries(MultiGameLauncher PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)
