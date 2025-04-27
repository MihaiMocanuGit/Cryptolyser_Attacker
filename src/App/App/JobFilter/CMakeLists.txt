add_library(JobFilter STATIC
        JobFilter.hpp JobFilter.cpp
)

target_include_directories(JobFilter
        PUBLIC
        "${PROJECT_SOURCE_DIR}/src/DataProcessing" "${PROJECT_SOURCE_DIR}/src/Study"
        "${PROJECT_SOURCE_DIR}/src/Filter"
        "${CMAKE_CURRENT_SOURCE_DIR}/../.."
)

target_link_libraries(JobFilter PUBLIC JobI DataProcessing Study Filter)
