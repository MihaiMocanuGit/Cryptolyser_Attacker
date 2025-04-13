add_library(DataVector STATIC
        DataVector.hpp DataVector.cpp
        DataVectorSerializer.hpp DataVectorSerializer.cpp
)

target_link_libraries(DataVector PUBLIC Serializer)
