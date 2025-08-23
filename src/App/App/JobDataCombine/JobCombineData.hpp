#pragma once

#include "../JobI/JobI.hpp"
#include "Study/Study.hpp"

#include <filesystem>
#include <format>
#include <string>
#include <vector>

namespace App
{

class JobCombineData : public JobI
{
  private:
    struct Input
    {
        std::vector<std::filesystem::path> loadPaths {};
        std::filesystem::path savePath {};
        bool onlyMetrics {false};
    } input;

  public:
    struct Buffers
    {
        std::vector<std::string> loadPaths {};
        std::string savePath {};
        bool onlyMetrics {false};
    };

    explicit JobCombineData(const Buffers &buffers);

    [[nodiscard]] ExitStatus_e invoke(std::stop_token stoken) override;

    [[nodiscard]] std::string description() const noexcept override;

    [[nodiscard]] std::unique_ptr<JobI> clone() const override;

    ~JobCombineData() override = default;
};
} // namespace App
