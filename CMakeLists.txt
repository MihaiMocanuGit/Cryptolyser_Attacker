cmake_minimum_required(VERSION 3.25)
project(Cryptolyser_Attacker VERSION 0.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

if (WIN32)
    FATAL_ERROR("Windows is not yet supported.")
endif (WIN32)

set(GCC_FLAGS_DEBUG "-O0 -Wall -Wextra -Wpedantic -Werror -Wno-unused-variable -Wno-unused-parameter -fsanitize=address,undefined")
set(GCC_LINK_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address,undefined")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${GCC_FLAGS_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${GCC_LINK_FLAGS_DEBUG}")

enable_testing()

add_subdirectory(third-party)
add_subdirectory(Cryptolyser_Common)
add_subdirectory(src)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE ServerConnection Cryptolyser_Common DataProcessing)
