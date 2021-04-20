cmake_minimum_required(VERSION 3.20.1)

set(BUILD_SHARED_LIBS ON)
project(nbody)

add_subdirectory(libs)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(nbody src/main.cpp src/app.cpp src/simulation.cpp)

target_include_directories(nbody PUBLIC libs/glad/include/ libs/glfw/include/ libs/quadtree/include/ libs/glm)
target_link_libraries(nbody glad quadtree glfw)
