set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

add_subdirectory(glad)
add_subdirectory(quadtree)
# GLM is header only so it doesn't need to be compiled
#add_subdirectory(glm)
