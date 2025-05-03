add_library(Widgets STATIC
        Widgets.hpp Widgets.cpp
)

target_include_directories(Widgets
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../.."
)

target_link_libraries(Widgets PUBLIC imgui nfd)
