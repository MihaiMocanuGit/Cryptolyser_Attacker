add_library(ServerConnection STATIC ServerConnection/ServerConnection.hpp ServerConnection/ServerConnection.cpp)
target_link_libraries(ServerConnection PUBLIC Cryptolyser_Common)
target_include_directories(ServerConnection
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
        PRIVATE "${PROJECT_SOURCE_DIR}/Cryptolyser_Common")
