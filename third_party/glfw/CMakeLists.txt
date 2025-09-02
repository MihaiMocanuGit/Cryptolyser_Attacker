Include(FetchContent)

FetchContent_Declare(
        glfw 
        GIT_REPOSITORY https://github.com/glfw/glfw
        GIT_TAG 3.4
        EXCLUDE_FROM_ALL
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)
