add_subdirectory(unit-tests)

add_library(Samples STATIC
        SampleMetrics.hpp SampleMetrics.cpp
        SampleData.hpp SampleData.cpp
        SampleGroup.cpp SampleGroup.hpp
)
