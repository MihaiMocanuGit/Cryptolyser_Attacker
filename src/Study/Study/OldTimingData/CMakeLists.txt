add_library(OldTimingData STATIC
        TimingData.hpp TimingData.cpp
)
target_include_directories(OldTimingData
        PRIVATE
        "${PROJECT_SOURCE_DIR}/src/DataProcessing" "${PROJECT_SOURCE_DIR}/src/ServerConnection"
        "${PROJECT_SOURCE_DIR}/Cryptolyser_Common")
target_link_libraries(OldTimingData PUBLIC DataProcessing ServerConnection Cryptolyser_Common)
