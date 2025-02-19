add_subdirectory(unit-tests)

add_library(DataProcessing STATIC
        SampleMetrics.hpp SampleMetrics.cpp
        SampleData.hpp SampleData.cpp
        TimingProcessing.hpp TimingProcessing.cpp
)
target_include_directories(DataProcessing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
