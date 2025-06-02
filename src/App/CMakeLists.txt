add_subdirectory(App/JobI)
add_subdirectory(App/JobCorrelate)
add_subdirectory(App/JobDataCombine)
add_subdirectory(App/JobFilter)
add_subdirectory(App/JobStudy)
add_subdirectory(App/WorkloadManager)

add_library(App INTERFACE)

target_link_libraries(App INTERFACE JobI JobCorrelate JobDataCombine JobFilter JobStudy WorkloadManager)

target_include_directories(App
        INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
