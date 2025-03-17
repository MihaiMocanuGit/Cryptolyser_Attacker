add_subdirectory(DataProcessing/Samples)
add_subdirectory(DataProcessing/Timings)

add_library(DataProcessing INTERFACE)
target_link_libraries(DataProcessing INTERFACE
        Samples
        Timings
)
target_include_directories(DataProcessing INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
