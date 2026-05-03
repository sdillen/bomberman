cmake_minimum_required(VERSION 3.21)
project(bomber LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(raylib REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ENET REQUIRED libenet)

file(GLOB SRC CONFIGURE_DEPENDS
  src/**/*.cpp src/*.cpp
)
add_executable(bomber ${SRC})
target_include_directories(bomber PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(bomber PRIVATE ${ENET_INCLUDE_DIRS})
target_link_directories(bomber PRIVATE ${ENET_LIBRARY_DIRS})
target_link_libraries(bomber PRIVATE raylib ${ENET_LIBRARIES})

