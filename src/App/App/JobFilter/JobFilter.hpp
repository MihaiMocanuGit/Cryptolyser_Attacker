#pragma once

#include "../JobI/JobI.hpp"
#include "Filter/Filter.hpp"
#include "Study/SerializerManager/SerializerManager.hpp"
#include "Study/Study.hpp"

#include <filesystem>
#include <format>
#include <string>

namespace App
{

class JobFilter : public JobI
{
  private:
    struct Input
    {
        double lb {}, ub {};
        std::filesystem::path savePath {};
        std::filesystem::path loadPath {};
    } input;

  public:
    struct Buffers
    {
        float lb {0.0f}, ub {0.0f};
        std::string savePath {};
        std::string loadPath {};
    };

    explicit JobFilter(const Buffers &buffers, const std::atomic_bool &continueRunning);

    void operator()() override;

    [[nodiscard]] std::string description() const noexcept override;

    [[nodiscard]] std::unique_ptr<JobI> clone() const override;

    ~JobFilter() override = default;
};
} // namespace App
