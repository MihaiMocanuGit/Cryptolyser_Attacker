add_library(TimingData STATIC
        TimingData.hpp TimingData.cpp
)
target_include_directories(TimingData
        PRIVATE
        "${PROJECT_SOURCE_DIR}/src/DataProcessing" "${PROJECT_SOURCE_DIR}/src/ServerConnection"
        "${PROJECT_SOURCE_DIR}/Cryptolyser_Common")
target_link_libraries(TimingData PUBLIC DataProcessing ServerConnection Cryptolyser_Common)
