add_subdirectory(DataProcessing/Samples)
add_subdirectory(DataProcessing/SampleData)
add_subdirectory(DataProcessing/MetricsData)
add_subdirectory(DataProcessing/DataVector)
add_subdirectory(DataProcessing/Timings)
add_subdirectory(DataProcessing/Distribution)

add_library(DataProcessing INTERFACE)
target_link_libraries(DataProcessing INTERFACE
        Samples
        SampleData
        MetricsData
        DataVector
        Timings
        Distribution
)
target_include_directories(DataProcessing INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
