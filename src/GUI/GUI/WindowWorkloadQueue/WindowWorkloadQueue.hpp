#pragma once

#include "../WindowI/WindowI.hpp"
#include "App/JobCorrelate/JobCorrelate.hpp"
#include "App/WorkloadManager/WorkloadManager.hpp"

namespace GUI
{
class WindowWorkloadQueue : public WindowI
{
  private:
    App::WorkloadManager &m_workloadManager;

    static std::ostringstream m_coutBuff;

    static void coutInit();

    static std::ostringstream m_cerrBuff;

    static void cerrInit();

  public:
    WindowWorkloadQueue(std::string_view name, App::WorkloadManager &workloadManager);

    void constructWindow() override;
};
} // namespace GUI
