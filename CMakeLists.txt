cmake_minimum_required(VERSION 3.10)

project(vktiny LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
# set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# subdirectory
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)
add_subdirectory(vulkan)
add_subdirectory(glslang)

# main
file(GLOB_RECURSE sources src/*.cpp)
file(GLOB_RECURSE headers include/vktiny/*.hpp)
add_library(vktiny STATIC ${sources} ${headers})

# link
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
target_link_libraries(${PROJECT_NAME} PUBLIC glslang)
target_link_libraries(${PROJECT_NAME} PUBLIC SPIRV)
target_link_libraries(${PROJECT_NAME} PUBLIC glslang-default-resource-limits)

# include
target_include_directories(${PROJECT_NAME} PUBLIC
                          "${PROJECT_SOURCE_DIR}/include"
                          "${PROJECT_SOURCE_DIR}/vulkan/include"
                          "${PROJECT_SOURCE_DIR}/glfw/include"
                          "${PROJECT_SOURCE_DIR}/glslang"
                          )

# examples
option(VKTINY_EXAMPLES "" OFF)
if(VKTINY_EXAMPLES)
    file(GLOB hello_compute_sources examples/src/hello_compute/*.cpp)
    add_executable(hello_compute ${hello_compute_sources})
    target_link_libraries(hello_compute vktiny)
    target_include_directories(hello_compute PUBLIC "${CMAKE_SOURCE_DIR}/include")

    # copy files
    file(COPY "${CMAKE_SOURCE_DIR}/examples/shader" DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY "${CMAKE_SOURCE_DIR}/examples/asset" DESTINATION ${CMAKE_BINARY_DIR})
endif()

