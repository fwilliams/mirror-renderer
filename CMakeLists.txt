cmake_minimum_required(VERSION 2.6)
project(RNDR)

set(CMAKE_CXX_COMPILER g++)

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "build")

set(
  CMAKE_MODULE_PATH 
  "${CMAKE_INSTALL_PREFIX}/share/cmake-modules" ${CMAKE_MODULE_PATH})
 
# Put shared libraries in lib directory
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# C++14 is rad
set(CMAKE_CXX_FLAGS "-g -std=c++14 -Wall ${CMAKE_CXX_FLAGS}")

include_directories(.)

add_subdirectory(geometry)
add_subdirectory(renderer)
add_subdirectory(utils)
add_subdirectory(tests)
enable_testing()

add_executable(generate_projected_tiles generate_projected_tiles.cpp)
target_link_libraries(generate_projected_tiles renderer util SOIL)

add_executable(virtual_copy_transform virtual_copy_transform.cpp)
target_link_libraries(virtual_copy_transform renderer util SOIL)

add_executable(composite composite.cpp)
target_link_libraries(composite renderer util SOIL)

add_executable(lumigraph_viewer lumigraph_viewer.cpp)
target_link_libraries(lumigraph_viewer renderer util SOIL boost_system boost_filesystem)

