cmake_minimum_required(VERSION 3.24)
project(DragonsAside)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb")

include_directories(include)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/vendor")

add_executable(dragons
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/board.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/tile.cpp"
)

target_link_libraries(dragons PRIVATE vendor)
