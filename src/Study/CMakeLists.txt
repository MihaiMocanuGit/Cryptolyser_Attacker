add_subdirectory(Study/TimingData)
add_subdirectory(Study/Gatherer)
add_subdirectory(Study/Logger)
add_subdirectory(Study/SerializerManager)

add_library(Study STATIC
        Study/Study.hpp Study/Study.cpp)
target_link_libraries(Study PUBLIC
        TimingData
        Gatherer
        Logger
        SerializerManager
)
target_include_directories(Study
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
