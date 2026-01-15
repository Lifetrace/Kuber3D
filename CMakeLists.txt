cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# label:
set(PROJECT_NAME Kuber)
project(${PROJECT_NAME}) 

add_executable(${PROJECT_NAME} src/main.cpp src/Engine.cpp src/Events.cpp src/Shader.cpp src/TetrahedronWire.cpp)

target_include_directories(Kuber PRIVATE ${CMAKE_SOURCE_DIR}/include)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_INSTALL OFF CACHE BOOL "" FORCE)


add_custom_command(TARGET Kuber POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:Kuber>/assets
)
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/OpenGL_includes)

foreach(dep glad glfw glm)
    add_subdirectory(${THIRD_PARTY_DIR}/${dep})
endforeach()

target_link_libraries(${PROJECT_NAME} PRIVATE glad glfw glm)
