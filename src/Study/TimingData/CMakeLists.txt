add_library(TimingData STATIC
        TimingData.hpp TimingData.cpp
)
target_include_directories(TimingData PRIVATE "${PROJECT_SOURCE_DIR}/src" "${PROJECT_SOURCE_DIR}/Cryptolyser_Common")
