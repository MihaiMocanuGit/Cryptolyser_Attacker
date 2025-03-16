add_library(Gatherer STATIC
        Gatherer.hpp Gatherer.cpp
)

target_include_directories(Gatherer PRIVATE "${PROJECT_SOURCE_DIR}/Cryptolyser_Common" "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(Gatherer PRIVATE Samples)
