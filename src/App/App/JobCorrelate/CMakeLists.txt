add_library(JobCorrelate STATIC
        JobCorrelate.hpp JobCorrelate.cpp
)

target_include_directories(JobCorrelate
        PRIVATE
        "${PROJECT_SOURCE_DIR}/src/DataProcessing" "${PROJECT_SOURCE_DIR}/src/Correlate"
        "${CMAKE_CURRENT_SOURCE_DIR}/../.."
)

target_link_libraries(JobCorrelate PUBLIC JobI DataProcessing Correlate)
