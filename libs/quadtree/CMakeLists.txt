add_library(quadtree src/quadtree.cpp)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g -pg")
set(CMAKE_CXX_FLAGS_RELEASE"-O3")

target_include_directories(quadtree PUBLIC ./include)

add_executable(test test/test.cpp)

enable_testing()
add_test(Tests test)


find_package(OpenMP REQUIRED)

target_link_libraries(test quadtree OpenMP::OpenMP_CXX)
