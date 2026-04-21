cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME Kuber3D)
project(${PROJECT_NAME})

add_executable(${PROJECT_NAME}
src/main.cpp
src/Window.cpp
src/Events.cpp
src/Shader.cpp
src/Buffers.cpp
src/Camera.cpp
src/Grid.cpp
src/ImGui_Style.cpp
src/imgui.cpp
src/imgui_draw.cpp
src/imgui_widgets.cpp
src/imgui_tables.cpp
src/imgui_impl_glfw.cpp
src/imgui_impl_opengl3.cpp
src/imgui_demo.cpp
src/Angle.cpp
src/Movement.cpp
src/Selecting.cpp
src/VectorMath.cpp
src/PointActions.cpp
src/Operations.cpp
src/Figures.cpp
)

if (MINGW)
  target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)

  get_filename_component(MINGW_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)

  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
      "$<TARGET_FILE_DIR:${PROJECT_NAME}>/libgcc_s_seh-1.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${MINGW_BIN_DIR}/libstdc++-6.dll"
      "$<TARGET_FILE_DIR:${PROJECT_NAME}>/libstdc++-6.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${MINGW_BIN_DIR}/libwinpthread-1.dll"
      "$<TARGET_FILE_DIR:${PROJECT_NAME}>/libwinpthread-1.dll"
  )
endif()


target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_INSTALL OFF CACHE BOOL "" FORCE)

set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/external)

foreach(dep glad glfw glm)
    add_subdirectory(${THIRD_PARTY_DIR}/${dep})
endforeach()

target_link_libraries(${PROJECT_NAME} PRIVATE glad glfw glm)

if (MINGW)
  target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
  target_link_options(${PROJECT_NAME} PRIVATE -Wl,-Bstatic -lwinpthread -Wl,-Bdynamic)
endif()

