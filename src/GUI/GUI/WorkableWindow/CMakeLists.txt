add_library(WorkableWindow STATIC
        WorkableWindow.hpp
)

set_target_properties(WorkableWindow PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(WorkableWindow
        PRIVATE
        "${PROJECT_SOURCE_DIR}/src/App"
        "${CMAKE_CURRENT_SOURCE_DIR}/../.."
)

target_link_libraries(WorkableWindow PUBLIC App)
