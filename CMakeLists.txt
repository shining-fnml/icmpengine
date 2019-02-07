project(plasma-icmp)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(QT_MIN_VERSION "5.4.0")
set(KF5_MIN_VERSION "5.9.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Network)
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS Plasma I18n)
# find_package(KF5Syndication)

include(KDECMakeSettings)
include(KDECompilerSettings)
include(KDEInstallDirs)
include(ECMPackageConfigHelpers)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DTRANSLATION_DOMAIN=\"plasma_engine_icmp\")

set(icmp_engine_SRCS
    icmpengine.cpp
)

LINK_LIBRARIES (pthread Qt5Network)

add_library(plasma_engine_icmp MODULE ${icmp_engine_SRCS})
kcoreaddons_desktop_to_json(plasma_engine_icmp plasma-engine-icmp.desktop)

target_link_libraries(plasma_engine_icmp
    KF5::Plasma
    KF5::I18n
)

install(TARGETS plasma_engine_icmp DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/dataengine)
install(FILES plasma-engine-icmp.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
