cmake_minimum_required(VERSION 3.16)

project(vktiny LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} samples/03_gltf_scene/03_gltf_scene.cpp)

find_package(glfw3 CONFIG REQUIRED)

set(VULKAN_LIB $ENV{VULKAN_SDK}/Lib)
set(VULKAN_INCLUDE $ENV{VULKAN_SDK}/Include)

target_link_libraries(${PROJECT_NAME} PUBLIC
    glfw
)

find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")

target_include_directories(${PROJECT_NAME} PUBLIC
    ${VULKAN_INCLUDE}
    ${PROJECT_SOURCE_DIR}
    ${TINYGLTF_INCLUDE_DIRS}
)
