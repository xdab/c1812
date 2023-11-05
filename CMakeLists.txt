cmake_minimum_required(VERSION 3.13)

project(c1812 VERSION 0.0.1 LANGUAGES C)

file(GLOB SOURCES src/*.c)
file(GLOB HEADERS include/*.h)

include_directories(include)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

add_executable(demo src/demo.c)
set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")
target_link_libraries(demo ${PROJECT_NAME})
target_link_libraries(demo m)

