add_library(WindowStudy STATIC
        WindowStudy.hpp WindowStudy.cpp
)

target_include_directories(WindowStudy
        PRIVATE
        "${PROJECT_SOURCE_DIR}/src/DataProcessing" "${PROJECT_SOURCE_DIR}/src/Study"
        "${PROJECT_SOURCE_DIR}/src/ServerConnection"
        "${PROJECT_SOURCE_DIR}/src/App"
        "${CMAKE_CURRENT_SOURCE_DIR}/../.."
        "${PROJECT_SOURCE_DIR}/Cryptolyser_Common"
)

target_link_libraries(WindowStudy PUBLIC DataProcessing Study ServerConnection App Cryptolyser_Common imgui)
