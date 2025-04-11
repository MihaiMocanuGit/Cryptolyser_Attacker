add_library(Correlate STATIC
        Correlate/Correlate.hpp Correlate/Correlate.cpp)
target_link_libraries(Correlate PUBLIC Study)
target_include_directories(Correlate
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
