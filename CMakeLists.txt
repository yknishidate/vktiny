cmake_minimum_required(VERSION 3.10)

project(vktiny LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
# set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# external
add_subdirectory(external)

# core lib
file(GLOB_RECURSE sources src/*.cpp)
file(GLOB_RECURSE headers include/vktiny/*.hpp)

add_library(vktiny STATIC ${sources} ${headers})
target_include_directories(vktiny PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(vktiny PUBLIC glm)
target_link_libraries(vktiny PUBLIC vulkan)
target_link_libraries(vktiny PUBLIC glfw)
target_link_libraries(vktiny PUBLIC spdlog)
target_link_libraries(vktiny PUBLIC tinygltf)

# examples
## raytracing_triangle
file(GLOB raytracing_triangle_sources examples/raytracing_triangle/*.cpp examples/raytracing_triangle/*.hpp)
add_executable(raytracing_triangle ${raytracing_triangle_sources})
target_link_libraries(raytracing_triangle vktiny)

## gltf_laoding
file(GLOB gltf_loading_sources examples/gltf_loading/*.cpp)
add_executable(gltf_loading ${gltf_loading_sources})
target_link_libraries(gltf_loading vktiny)

# copy files
file(COPY "${CMAKE_SOURCE_DIR}/examples/shader" DESTINATION ${CMAKE_BINARY_DIR})
file(COPY "${CMAKE_SOURCE_DIR}/examples/asset" DESTINATION ${CMAKE_BINARY_DIR})
