#pragma once

#include "../JobI/JobI.hpp"
#include "Study/Study.hpp"

#include <filesystem>
#include <format>
#include <string>
#include <vector>

namespace App
{
struct BuffersCombineData
{
    std::vector<std::string> loadPaths {};
    std::string savePath {};
    bool onlyMetrics {false};
};

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
    explicit JobCombineData(const BuffersCombineData &buffers,
                            const std::atomic_flag &continueRunning);

    void operator()() override;

    [[nodiscard]] std::string description() const noexcept override;

    ~JobCombineData() override = default;
};
} // namespace App
