cmake_minimum_required(VERSION 4.1.1 FATAL_ERROR)

# label:
set(PROJECT_NAME Kuber)
project(${PROJECT_NAME}) 

project(${PROJECT_NAME})

add_executable(${PROJECT_NAME} src/main.cpp)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(include/glfw)
target_link_libraries(${PROJECT_NAME} glfw)

add_subdirectory(include/glad)
target_link_libraries(${PROJECT_NAME} glad)