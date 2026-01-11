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

    # --- Launcher ---
    Launcher/launcher.h
    Launcher/launcher.cpp

    # --- Ogolne ---
    Ogolne/game_config.h
    Ogolne/zasoby.qrc

    # --- Wisielec ---
    GraWisielec/wisielec_window.h
    GraWisielec/wisielec_window.cpp
    GraWisielec/game_logic.h
    GraWisielec/game_logic.cpp

    # --- Kosci ---
    GraKosci/kosci_window.h
    GraKosci/kosci_window.cpp
    GraKosci/kosci_logic.h
    GraKosci/kosci_logic.cpp
    GraKosci/kosci_network.h
    GraKosci/kosci_network.cpp
    GraKosci/kosci_config.h
    GraKosci/oknogry.ui

    # --- Chinczyk (Ludo) ---
    GraLudo/chinczyk_window.cpp
    GraLudo/chinczyk_window.h
    GraLudo/boardscene.cpp
    GraLudo/boardscene.h
    GraLudo/tokenitem.cpp
    GraLudo/tokenitem.h
    GraLudo/gra.cpp
    GraLudo/gra.h
    GraLudo/gracz.cpp
    GraLudo/gracz.h
    GraLudo/pionek.cpp
    GraLudo/pionek.h
    GraLudo/plansza.cpp
    GraLudo/plansza.h
    GraLudo/kostka.cpp
    GraLudo/kostka.h
    GraLudo/chinczyk_network.cpp
    GraLudo/chinczyk_network.h
)

target_include_directories(MultiGameLauncher PRIVATE
    ${CMAKE_SOURCE_DIR}/Ogolne
    ${CMAKE_SOURCE_DIR}/Launcher
    ${CMAKE_SOURCE_DIR}/GraWisielec
    ${CMAKE_SOURCE_DIR}/GraKosci
    ${CMAKE_SOURCE_DIR}/GraLudo
)

target_link_libraries(MultiGameLauncher PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)
