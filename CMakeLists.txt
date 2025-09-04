cmake_minimum_required(VERSION 3.25)
project(
  Cryptolyser_Attacker
  VERSION 1.1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

if(WIN32)
  fatal_error("Windows is not yet supported.")
endif(WIN32)

set(ERRORS "-Wall -Wextra -Wpedantic")
set(WARNINGS
    "-Wno-error=unused-variable -Wno-error=unused-parameter -Wno-error=unused-function -Wno-error=unused-but-set-variable"
)
set(SANITIZERS "-fsanitize=address,undefined")

set(GCC_FLAGS_DEBUG "-O0 -ggdb3 ${ERRORS} ${WARNINGS} ${SANITIZERS}")
set(GCC_LINK_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${SANITIZERS}")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${GCC_FLAGS_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG
    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${GCC_LINK_FLAGS_DEBUG} ")

# Useful for nvim/emacs setups
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
execute_process(
  COMMAND
    ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json)

enable_testing()
add_executable(unit_tests)

add_subdirectory(third_party)
add_subdirectory(Cryptolyser_Common)
add_subdirectory(src)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE Cryptolyser_Common src
                                              third_party)

target_link_options(unit_tests PRIVATE -ltbb)
target_link_libraries(unit_tests PRIVATE third_party src)
catch_discover_tests(unit_tests)
