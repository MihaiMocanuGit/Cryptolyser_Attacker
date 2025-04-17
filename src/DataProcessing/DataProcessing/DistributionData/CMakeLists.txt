add_library(DistributionData STATIC
        DistributionData.hpp DistributionData.cpp
)

target_link_options(DistributionData PUBLIC -ltbb)

target_link_libraries(DistributionData PUBLIC Serializer)
