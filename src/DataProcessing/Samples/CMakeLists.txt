add_subdirectory(unit-tests)

add_library(Samples STATIC
        SampleMetrics.hpp SampleMetrics.cpp
        SampleData.hpp SampleData.cpp
        SampleGroup.cpp SampleGroup.hpp
)
#target_include_directories(Samples PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
