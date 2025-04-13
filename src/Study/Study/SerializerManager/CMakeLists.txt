add_library(SerializerManager STATIC
        SerializerManager.hpp SerializerManager.cpp
)
target_include_directories(SerializerManager
        PRIVATE
        "${PROJECT_SOURCE_DIR}/src/DataProcessing" "${CMAKE_CURRENT_SOURCE_DIR}/../.."
)
target_link_libraries(SerializerManager PUBLIC DataProcessing TimingData)
