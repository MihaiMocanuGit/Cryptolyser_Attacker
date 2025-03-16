add_subdirectory(TimingData)
add_subdirectory(Gatherer)
add_subdirectory(Logger)
add_subdirectory(Saver)

add_library(Study STATIC
        Study.hpp Study.cpp)
target_link_libraries(Study INTERFACE
        TimingData
        Gatherer
        Logger
        Saver
)
