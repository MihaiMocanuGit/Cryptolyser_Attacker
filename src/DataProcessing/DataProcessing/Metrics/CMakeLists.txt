add_subdirectory(unit-tests)

add_library(Metrics STATIC
        Metrics.hpp Metrics.cpp
        SampleData.hpp SampleData.cpp
        SampleGroup.cpp SampleGroup.hpp
        MetricsSerializer.cpp MetricsSerializer.hpp
)

target_link_libraries(Metrics PUBLIC Serializer)
