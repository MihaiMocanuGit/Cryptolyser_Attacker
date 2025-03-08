add_subdirectory(Samples)
add_subdirectory(Timings)

add_library(DataProcessing INTERFACE)
target_link_libraries(DataProcessing INTERFACE
        Samples
        Timings
)
