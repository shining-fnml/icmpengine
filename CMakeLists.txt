#  ** Edit the next two line to suit your needs **
set(PLUGIN_INSTALL_DIR /usr/lib/kde4)
set(SERVICES_INSTALL_DIR /usr/share/kde4/services)

# ** Don't touch anything after this line, unless you know what you are doing **
project(plasma-icmp)
 
# Find the required Libaries
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )
 
# We add our source code here
set(icmp_engine_SRCS icmpengine.cpp)
LINK_LIBRARIES (pthread)
 

# Now make sure all files get to the right place
kde4_add_plugin(plasma_engine_icmp ${icmp_engine_SRCS})
target_link_libraries(plasma_engine_icmp
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_PLASMA_LIBS})
 

install(TARGETS plasma_engine_icmp
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-engine-icmp.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
