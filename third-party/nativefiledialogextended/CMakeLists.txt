Include(FetchContent)

FetchContent_Declare(
        nfd
        GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended
        GIT_TAG v1.2.1
        OVERRIDE_FIND_PACKAGE
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(nfd)

target_include_directories(nfd PUBLIC ${nfd_SOURCE_DIR})
