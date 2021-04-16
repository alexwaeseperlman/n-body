add_library(quadtree src/quadtree.cpp)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O2")
set(CMAKE_CXX_FLAGS_RELEASE"-O3")

target_include_directories(quadtree PUBLIC ./include)

add_executable(test test/test.cpp)
target_link_libraries(test quadtree)

enable_testing()
add_test(Tests test)


