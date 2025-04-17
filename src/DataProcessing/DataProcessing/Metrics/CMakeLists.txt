add_library(Metrics STATIC
        Metrics.hpp Metrics.cpp
        MetricsSerializer.cpp MetricsSerializer.hpp
)

target_link_libraries(Metrics PUBLIC Serializer)
