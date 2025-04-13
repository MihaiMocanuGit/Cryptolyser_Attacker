add_library(DistributionData STATIC
        DistributionData.hpp DistributionData.cpp
        DistributionByteValue.hpp DistributionByteValue.cpp
        DistributionByteBlock.hpp DistributionByteBlock.cpp
)

target_link_options(DistributionData PUBLIC -ltbb) # Note, delete this after removing the old classes

target_link_libraries(DistributionData PUBLIC Serializer)
