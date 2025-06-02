#pragma once

#include <atomic>
#include <memory>
#include <string>

namespace App
{
class JobI
{
  protected:
    const std::atomic_bool &m_continueRunning;

    struct Input;

  public:
    struct Buffers;

    explicit JobI(const std::atomic_bool &continueRunning) : m_continueRunning {continueRunning} {}

    virtual void operator()() = 0;

    [[nodiscard]] virtual std::string description() const noexcept = 0;

    [[nodiscard]] virtual std::unique_ptr<JobI> clone() const = 0;

    virtual ~JobI() = default;
};
} // namespace App
