add_library(quadtree quadtree.cpp)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O2")
set(CMAKE_CXX_FLAGS_RELEASE"-O3")


add_executable(test test.cpp)
target_link_libraries(test quadtree)

enable_testing()
add_test(Tests test)

target_include_directories(quadtree PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

