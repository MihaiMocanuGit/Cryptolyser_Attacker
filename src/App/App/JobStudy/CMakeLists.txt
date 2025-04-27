add_library(JobStudy STATIC
        JobStudy.hpp JobStudy.cpp
)

target_include_directories(JobStudy
        PRIVATE
        "${PROJECT_SOURCE_DIR}/src/DataProcessing" "${PROJECT_SOURCE_DIR}/src/Study"
        "${PROJECT_SOURCE_DIR}/src/ServerConnection"
        "${CMAKE_CURRENT_SOURCE_DIR}/../.."
        "${PROJECT_SOURCE_DIR}/Cryptolyser_Common"
)

target_link_libraries(JobStudy PUBLIC JobI DataProcessing Study ServerConnection Cryptolyser_Common)
