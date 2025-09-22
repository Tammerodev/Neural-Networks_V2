cmake_minimum_required(VERSION 3.22)
project(NeuralNetworks VERSION 2.0)

include(FetchContent)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

# Compiler flags
set(CMAKE_C_FLAGS_RELEASE "-O3")

find_package(OpenGL REQUIRED)

FetchContent_Declare(
  SFML
  URL "https://github.com/SFML/SFML/archive/3.0.2.zip"
)

FetchContent_Declare(
  imgui
  URL "https://github.com/ocornut/imgui/archive/refs/tags/v1.91.1.zip"
)

FetchContent_Declare(
  imgui-sfml
  GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
  GIT_COMMIT 5f54b69b6aee18db846c81633f86f78c2586dded
  # ^ or like this - sometimes it's better because FetchContent won't look
  # into remote to see if branch head was updated or not - good for stable
  # tags like 'vX.X' corresponding to releases
)

FetchContent_MakeAvailable(sfml)
FetchContent_MakeAvailable(imgui)

# Tell imgui-sfml where ImGui is
set(IMGUI_DIR ${imgui_SOURCE_DIR})
option(IMGUI_SFML_FIND_SFML "Use find_package to find SFML" ON)
option(IMGUI_SFML_IMGUI_DEMO "Build imgui_demo.cpp" OFF)

# Now make imgui-sfml available
FetchContent_MakeAvailable(imgui-sfml)

# Then link everything
target_link_libraries(${PROJECT_NAME} PRIVATE
    sfml-system
    sfml-window
    sfml-graphics
    sfml-network
    sfml-audio
    ImGui-SFML::ImGui-SFML
    OpenGL::GL
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}          # for imgui.h
    ${imgui-sfml_SOURCE_DIR}
)
