add_library(Study STATIC
        Study.hpp Study.cpp
        StudyDoppelganger.hpp StudyDoppelganger.cpp
)

target_link_libraries(Study PRIVATE src)
