add_library(Gatherer STATIC
        Gatherer.hpp Gatherer.cpp
)

target_include_directories(Gatherer
        PRIVATE
        "${PROJECT_SOURCE_DIR}/Cryptolyser_Common" "${PROJECT_SOURCE_DIR}/src/DataProcessing"
        "${PROJECT_SOURCE_DIR}/src/ServerConnection" "${CMAKE_CURRENT_SOURCE_DIR}/../..")
target_link_libraries(Gatherer PUBLIC Cryptolyser_Common DataProcessing ServerConnection TimingData)
