#pragma once

#include <string>
#include <string_view>

namespace GUI
{
class WindowI
{
  protected:
    std::string m_name {};

  public:
    explicit WindowI(std::string_view name);

    virtual void constructWindow() = 0;

    virtual std::string_view name() const;
};
} // namespace GUI
