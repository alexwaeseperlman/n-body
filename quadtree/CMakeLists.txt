add_library(quadtree quadtree.cpp)

set(CMAKE_BUILD_TYPE Debug)

add_executable(test test.cpp)
target_link_libraries(test quadtree)

enable_testing()
add_test(Tests test)

target_include_directories(quadtree PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

