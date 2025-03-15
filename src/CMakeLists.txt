add_subdirectory(DataProcessing)
add_subdirectory(ServerConnection)
add_subdirectory(Study)
add_subdirectory(Correlate)

add_library(src INTERFACE)
target_link_libraries(src INTERFACE DataProcessing ServerConnection Study Correlate)
target_include_directories(src INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
