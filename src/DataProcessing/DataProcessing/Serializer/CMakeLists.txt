add_library(Serializer INTERFACE Serializer.hpp)

target_include_directories(Serializer INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Serializer INTERFACE
        SampleData Metrics MetricsData DistributionData DataVector)
