file(GLOB_RECURSE utils_srcs ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(util SHARED ${utils_srcs})
target_link_libraries(util GL)
target_link_libraries(util GLEW)
target_link_libraries(util SDL2)