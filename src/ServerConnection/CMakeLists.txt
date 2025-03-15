add_library(ServerConnection STATIC
        ServerConnection.hpp ServerConnection.cpp
        ServerKeyConnection.hpp ServerKeyConnection.cpp)
target_link_libraries(ServerConnection PUBLIC Cryptolyser_Common)
