add_library(WindowWorkloadQueue STATIC
        WindowWorkloadQueue.hpp WindowWorkloadQueue.cpp
)

target_include_directories(WindowWorkloadQueue
        PRIVATE
        "${PROJECT_SOURCE_DIR}/src/DataProcessing"
        "${PROJECT_SOURCE_DIR}/src/Study"
        "${PROJECT_SOURCE_DIR}/src/ServerConnection"
        "${PROJECT_SOURCE_DIR}/src/App"
        "${CMAKE_CURRENT_SOURCE_DIR}/../.."
        "${PROJECT_SOURCE_DIR}/Cryptolyser_Common"
)

target_link_libraries(WindowWorkloadQueue PUBLIC DataProcessing Study ServerConnection App Cryptolyser_Common imgui)
