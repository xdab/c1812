cmake_minimum_required(VERSION 3.13)
project(c1812 VERSION 0.0.1 LANGUAGES C)

file(GLOB LIB_SOURCES src/*.c)
file(GLOB LIB_HEADERS include/*.h)
include_directories(include)
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${LIB_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

file(GLOB CLI_SOURCES cli/*.c)
file(GLOB CLI_HEADERS lib/*.h)
add_executable(cli ${CLI_SOURCES} ${CLI_HEADERS})
set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")
target_link_libraries(cli ${PROJECT_NAME})
target_link_libraries(cli m)

