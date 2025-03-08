add_subdirectory(DataProcessing)
add_subdirectory(ServerConnection)

add_library(src INTERFACE)
target_link_libraries(src INTERFACE DataProcessing ServerConnection)
target_include_directories(src INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
