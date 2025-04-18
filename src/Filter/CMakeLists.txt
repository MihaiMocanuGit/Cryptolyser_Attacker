add_library(Filter STATIC
        Filter/Filter.hpp Filter/Filter.cpp)
target_link_libraries(Filter PUBLIC DataProcessing)
target_include_directories(Filter
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
