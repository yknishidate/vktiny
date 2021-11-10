cmake_minimum_required(VERSION 3.10)

project(external LANGUAGES CXX)

# glm
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glm)

# vulkan
add_library(vulkan INTERFACE)
target_include_directories(vulkan INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/include)

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

# spdlog
add_library(spdlog INTERFACE)
target_include_directories(spdlog INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include)

# tinygltf
# add_library(tinygltf INTERFACE)
# target_include_directories(tinygltf INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tinygltf)

add_library(tinygltf INTERFACE)
set(TINYGLTF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tinygltf)
target_sources(tinygltf INTERFACE ${TINYGLTF_DIR}/tiny_gltf.h ${TINYGLTF_DIR}/json.hpp)
target_include_directories(tinygltf INTERFACE ${TINYGLTF_DIR})
