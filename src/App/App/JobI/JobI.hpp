#pragma once

#include <atomic>
#include <string>

namespace App
{
class JobI
{
  protected:
    const std::atomic_flag &m_continueRunning;

  public:
    explicit JobI(const std::atomic_flag &continueRunning) : m_continueRunning {continueRunning} {}

    virtual void operator()() = 0;

    [[nodiscard]] virtual std::string description() const noexcept = 0;

    virtual ~JobI() = default;
};
} // namespace App
