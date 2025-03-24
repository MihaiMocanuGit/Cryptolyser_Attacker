add_library(Distribution STATIC
        DistributionByteValue.hpp DistributionByteValue.cpp
        DistributionByteBlock.hpp DistributionByteBlock.cpp
)

target_link_options(Distribution PUBLIC -ltbb)
