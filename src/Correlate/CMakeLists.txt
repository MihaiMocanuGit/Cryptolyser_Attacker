add_library(Correlate STATIC
        Correlate.hpp Correlate.cpp
)

target_link_libraries(Correlate PRIVATE src)
