add_subdirectory(Study/OldTimingData)
add_subdirectory(Study/TimingData)
add_subdirectory(Study/Gatherer)
add_subdirectory(Study/Logger)
add_subdirectory(Study/SaveLoad)

add_library(Study STATIC
        Study/Study.hpp Study/Study.cpp)
target_link_libraries(Study PUBLIC
        OldTimingData
        TimingData
        Gatherer
        Logger
        SaveLoad
)
target_include_directories(Study
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
