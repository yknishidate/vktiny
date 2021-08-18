cmake_minimum_required(VERSION 3.10)

project(vktiny LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
# set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# external
add_subdirectory(external)

# core lib
file(GLOB sources src/*.cpp)
file(GLOB headers include/vktiny/*.hpp)

add_library(vktiny STATIC ${sources} ${headers})
target_include_directories(vktiny PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(vktiny PUBLIC glm)
target_link_libraries(vktiny PUBLIC vulkan)
target_link_libraries(vktiny PUBLIC glfw)
target_link_libraries(vktiny PUBLIC spdlog)
target_link_libraries(vktiny PUBLIC tinygltf)

# examples
file(GLOB raytracing_triangle_sources examples/raytracing_triangle/*.cpp examples/raytracing_triangle/*.hpp)
add_executable(raytracing_triangle ${raytracing_triangle_sources})
target_link_libraries(raytracing_triangle vktiny)

file(COPY "${CMAKE_SOURCE_DIR}/shader" DESTINATION ${CMAKE_BINARY_DIR})
# file(COPY "${CMAKE_SOURCE_DIR}/Asset" DESTINATION ${CMAKE_BINARY_DIR})