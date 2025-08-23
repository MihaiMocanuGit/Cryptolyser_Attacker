#pragma once

#include <atomic>
#include <memory>
#include <stop_token>
#include <string>

namespace App
{
class JobI
{
  protected:
    struct Input;

  public:
    struct Buffers;

    JobI() = default;

    enum class ExitStatus_e
    {
        OK,
        KILLED,
    };

    [[nodiscard]] virtual ExitStatus_e invoke(std::stop_token stoken) = 0;

    [[nodiscard]] virtual std::string description() const noexcept = 0;

    [[nodiscard]] virtual std::unique_ptr<JobI> clone() const = 0;

    virtual ~JobI() = default;
};
} // namespace App
