add_library(Logger STATIC
        Logger.hpp Logger.cpp
)

target_include_directories(Logger
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../..")
target_link_libraries(Logger PUBLIC Gatherer)
