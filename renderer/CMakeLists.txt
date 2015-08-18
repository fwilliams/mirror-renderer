file(GLOB_RECURSE gfx_srcs ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(renderer SHARED ${gfx_srcs})
target_link_libraries(renderer GL)
target_link_libraries(renderer GLEW)
