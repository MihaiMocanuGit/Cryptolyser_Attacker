add_subdirectory(GUI/WindowI)
add_subdirectory(GUI/WindowWorkloadQueue)
add_subdirectory(GUI/WindowCorrelate)
add_subdirectory(GUI/WindowDataCombine)
add_subdirectory(GUI/WindowFilter)
add_subdirectory(GUI/WindowStudy)

add_library(GUI STATIC
        GUI/GUI.hpp GUI/GUI.cpp
        GUI/Helpers.hpp
)

target_link_libraries(GUI PUBLIC
        WindowI
        WindowWorkloadQueue
        WindowCorrelate
        WindowDataCombine
        WindowFilter
        WindowStudy
)

target_include_directories(GUI
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
