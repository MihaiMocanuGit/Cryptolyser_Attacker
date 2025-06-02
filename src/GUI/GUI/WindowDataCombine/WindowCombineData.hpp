#pragma once

#include "../WindowI/WindowI.hpp"
#include "../WorkableWindow/WorkableWindow.hpp"
#include "App/JobDataCombine/JobCombineData.hpp"

namespace GUI
{
class WindowCombineData : public WindowI, public WorkableWindow
{
  private:
    App::JobCombineData::Buffers m_buffers {};

  public:
    WindowCombineData(std::string_view name, App::WorkloadManager &workloadManager);

    [[nodiscard]] std::unique_ptr<App::JobI> job() const override;

    void constructWindow() override;
};
} // namespace GUI
