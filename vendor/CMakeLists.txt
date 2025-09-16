set(BUILD_SHARED_LIBS OFF)
set(SDL_TEST OFF)
add_subdirectory(SDL)
add_subdirectory(SDL_ttf)

add_library(vendor INTERFACE)
target_link_libraries(vendor INTERFACE SDL3_ttf::SDL3_ttf SDL3::SDL3)
