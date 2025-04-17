add_subdirectory(unit-tests)

add_library(SampleData STATIC
        SampleData.hpp SampleData.cpp
        SampleDataSerializer.hpp SampleDataSerializer.cpp
)

target_link_libraries(SampleData PUBLIC Serializer)
