add_library(WorkloadManager STATIC
        WorkloadManager.hpp WorkloadManager.cpp
        NewWorkloadManager.cpp
        NewWorkloadManager.hpp
)

target_include_directories(WorkloadManager
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../.."
)

target_link_libraries(WorkloadManager PUBLIC JobI)
