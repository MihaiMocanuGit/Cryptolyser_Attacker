add_subdirectory(Catch2)
add_subdirectory(SDL2)
add_subdirectory(imgui)
add_subdirectory(nativefiledialogextended)

add_library(third_party INTERFACE)

target_link_libraries(third_party INTERFACE Catch2::Catch2WithMain imgui
                                            SDL2::SDL2 nfd)
target_include_directories(third_party INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
