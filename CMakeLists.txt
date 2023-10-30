cmake_minimum_required(VERSION 3.0)
project(c1812 VERSION 1.0.1 DESCRIPTION "c1812 description")
include(GNUInstallDirs)
add_library(c1812 SHARED src/c1812.c)
set_target_properties(c1812 PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER include/c1812.h)
configure_file(c1812.pc.in c1812.pc @ONLY)
target_include_directories(c1812 PRIVATE ./include)
install(TARGETS c1812
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_BINARY_DIR}/c1812.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)