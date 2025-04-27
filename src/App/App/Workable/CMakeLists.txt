add_library(Workable STATIC
        Workable.hpp
)

set_target_properties(Workable PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(Workable
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../.."
)

target_link_libraries(Workable PUBLIC JobI)
