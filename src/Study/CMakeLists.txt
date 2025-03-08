add_library(Study STATIC
        Study.hpp Study.cpp
)

target_link_libraries(Study PRIVATE src)
