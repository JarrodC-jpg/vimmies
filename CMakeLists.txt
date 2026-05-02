cmake_minimum_required(VERSION 3.20)

project(Stickies VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(Stickies
  src/main.cpp
  src/MainWindow.cpp
  src/Editor.cpp
  src/ModeIndicator.cpp
  src/CommandModule.cpp
  src/CommandModule.cpp
  src/ProjectLabelModule.cpp
)

target_link_libraries(Stickies PRIVATE Qt6::Core Qt6::Widgets)

# Nice warnings and optimizations
target_compile_options(Stickies PRIVATE -Wall -Wextra -pedantic)

set_target_properties(Stickies PROPERTIES 
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
