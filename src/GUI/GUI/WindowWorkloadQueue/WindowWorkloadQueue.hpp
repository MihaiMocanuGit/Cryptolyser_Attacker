#pragma once

#include "../WindowI/WindowI.hpp"
#include "App/JobCorrelate/JobCorrelate.hpp"
#include "App/WorkloadManager/NewWorkloadManager.hpp"

namespace GUI
{
class WindowWorkloadQueue : public WindowI
{
  private:
    App::NewWorkloadManager &m_workloadManager;

    static std::ostringstream m_coutBuff;

    static void coutInit();

    static std::ostringstream m_cerrBuff;

    static void cerrInit();

  public:
    WindowWorkloadQueue(std::string_view name, App::NewWorkloadManager &workloadManager);

    void constructWindow() override;
};
} // namespace GUI
