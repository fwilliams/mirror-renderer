file(GLOB_RECURSE renderer_srcs ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(renderer SHARED ${renderer_srcs})
target_link_libraries(renderer GL)
target_link_libraries(renderer GLEW)
