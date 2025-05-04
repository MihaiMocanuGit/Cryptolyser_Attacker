#pragma once

#include "../WindowI/WindowI.hpp"
#include "App/JobDataCombine/JobCombineData.hpp"
#include "App/Workable/Workable.hpp"

namespace GUI
{
class WindowCombineData : public WindowI, public App::Workable
{
  private:
    App::BuffersCombineData m_buffers {};

  public:
    WindowCombineData(std::string_view name, App::WorkloadManager &workloadManager);

    [[nodiscard]] std::unique_ptr<App::JobI> job() const override;

    void constructWindow() override;
};
} // namespace GUI
