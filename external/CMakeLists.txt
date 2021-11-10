cmake_minimum_required(VERSION 3.10)

project(external LANGUAGES CXX)

# subdirectory
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)
add_subdirectory(vulkan)
add_subdirectory(glslang)

# add_library(glfw PUBLIC)
# add_library(vulkan INTERFACE)
# add_library(glslang PUBLIC)

# # main
# add_library(external STATIC)

# # link
# target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
# target_link_libraries(${PROJECT_NAME} PUBLIC glslang)
# target_link_libraries(${PROJECT_NAME} PUBLIC SPIRV)

# # include
# target_include_directories(${PROJECT_NAME} PUBLIC
#                           "${PROJECT_BINARY_DIR}"
#                           "${PROJECT_SOURCE_DIR}/vulkan/include"
#                           "${PROJECT_SOURCE_DIR}/glfw/include"
#                           "${PROJECT_SOURCE_DIR}/glslang"
#                           )
