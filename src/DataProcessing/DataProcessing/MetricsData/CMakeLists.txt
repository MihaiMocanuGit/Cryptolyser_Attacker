add_library(MetricsData STATIC
        MetricsData.hpp MetricsData.cpp
        MetricsDataSerializer.hpp MetricsDataSerializer.cpp
)

target_link_libraries(MetricsData PUBLIC Serializer)
