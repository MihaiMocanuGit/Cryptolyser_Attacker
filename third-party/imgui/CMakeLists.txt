Include(FetchContent)

FetchContent_Declare(
        imgui_git 
        GIT_REPOSITORY https://github.com/ocornut/imgui
        GIT_TAG v1.91.9b-docking
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(imgui_git)

add_library(imgui
	${imgui_git_SOURCE_DIR}/imgui.cpp
	${imgui_git_SOURCE_DIR}/imgui_draw.cpp
	${imgui_git_SOURCE_DIR}/imgui_tables.cpp
	${imgui_git_SOURCE_DIR}/imgui_widgets.cpp
	${imgui_git_SOURCE_DIR}/imgui_demo.cpp
	${imgui_git_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
	${imgui_git_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
)
find_package(OpenGL REQUIRED)
# SDL2::SDL2main may or may not be available. It is e.g. required by Windows GUI applications
if(TARGET SDL2::SDL2main)
    # It has an implicit dependency on SDL2 functions, so it MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
	target_link_libraries(imgui PUBLIC SDL2::SDL2main)
endif()
target_link_libraries(imgui PUBLIC SDL2::SDL2 OpenGL::GL)
target_include_directories(imgui PUBLIC ${imgui_git_SOURCE_DIR})
