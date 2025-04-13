add_subdirectory(DataProcessing/Metrics)
add_subdirectory(DataProcessing/SampleData)
add_subdirectory(DataProcessing/MetricsData)
add_subdirectory(DataProcessing/DataVector)
add_subdirectory(DataProcessing/Timings)
add_subdirectory(DataProcessing/Serializer)
add_subdirectory(DataProcessing/DistributionData)

add_library(DataProcessing INTERFACE)
target_link_libraries(DataProcessing INTERFACE
        Metrics
        SampleData
        Serializer
        MetricsData
        DataVector
        Timings
        DistributionData
)
target_include_directories(DataProcessing INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
