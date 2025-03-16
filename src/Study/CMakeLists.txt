add_subdirectory(GatheredTimings)
add_subdirectory(Gatherer)
add_subdirectory(Logger)
add_subdirectory(Saver)

add_library(Study STATIC
        Study.hpp Study.cpp)
target_link_libraries(Study INTERFACE
        GatheredTimings
        Gatherer
        Logger
        Saver
)
