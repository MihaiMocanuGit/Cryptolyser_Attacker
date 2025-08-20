add_library(JobScheduler STATIC JobScheduler.cpp JobScheduler.hpp)

target_include_directories(JobScheduler
                           PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../..")

target_link_libraries(JobScheduler PUBLIC JobI)
