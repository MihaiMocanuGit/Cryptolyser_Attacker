add_library(SaveLoad STATIC
        SaveLoad.hpp SaveLoad.cpp
)

target_include_directories(SaveLoad
        PRIVATE
        "${PROJECT_SOURCE_DIR}/src/DataProcessing" "${CMAKE_CURRENT_SOURCE_DIR}/../..")
target_link_libraries(SaveLoad PUBLIC DataProcessing TimingData)
