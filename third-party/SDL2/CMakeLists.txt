Include(FetchContent)

FetchContent_Declare(
        SDL2 
        GIT_REPOSITORY https://github.com/libsdl-org/SDL
        GIT_TAG release-2.32.4
        OVERRIDE_FIND_PACKAGE
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(SDL2)
