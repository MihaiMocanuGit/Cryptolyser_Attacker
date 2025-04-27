add_library(JobDataCombine STATIC
        JobCombineData.hpp JobCombineData.cpp
)

target_include_directories(JobDataCombine
        PRIVATE
        "${PROJECT_SOURCE_DIR}/src/DataProcessing" "${PROJECT_SOURCE_DIR}/src/Study"
        "${CMAKE_CURRENT_SOURCE_DIR}/../.."
)

target_link_libraries(JobDataCombine PUBLIC JobI DataProcessing Study)
