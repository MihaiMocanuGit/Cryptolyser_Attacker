#pragma once
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include <filesystem>
#include <string>

namespace GUI::Widgets
{

const std::string fileExplorerWidget_defaultPath {std::filesystem::current_path().string()};

void fileExplorerWidget(std::string &r_path, std::string_view textLabel,
                        std::string_view buttonLabel = "Search");

} // namespace GUI::Widgets
