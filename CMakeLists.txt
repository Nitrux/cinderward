cmake_minimum_required(VERSION 3.16)

project(cinderward VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QT_MAJOR_VERSION 6)

# === 1. COMPILER FLAGS (x86-64-v3) ===
if((CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    add_compile_options(-march=x86-64-v3)
endif()

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECompilerSettings)
include(ECMFindQmlModule)
include(ECMSetupVersion)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 DBus)
find_package(KF6 REQUIRED COMPONENTS CoreAddons I18n WindowSystem)
find_package(MauiKit4 REQUIRED)

ecm_find_qmlmodule(org.mauikit.controls 1.0)

# === 2. RENAMED EXECUTABLE ===
add_executable(cinderward
    src/main.cpp
    src/firewallbackend.cpp
    src/firewallbackend.h
    resources.qrc
)

target_link_libraries(cinderward PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::DBus
    KF6::CoreAddons
    KF6::I18n
    KF6::WindowSystem
    MauiKit4
)

install(TARGETS cinderward ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
