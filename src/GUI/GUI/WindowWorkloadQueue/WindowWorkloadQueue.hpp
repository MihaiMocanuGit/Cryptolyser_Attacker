#pragma once

#include "../WindowI/WindowI.hpp"
#include "App/JobScheduler/JobScheduler.hpp"

namespace GUI
{
class WindowWorkloadQueue : public WindowI
{
  private:
    App::JobScheduler &m_jobScheduler;

    static std::ostringstream m_coutBuff;

    static void coutInit();

    static std::ostringstream m_cerrBuff;

    static void cerrInit();

  public:
    WindowWorkloadQueue(std::string_view name, App::JobScheduler &jobScheduler);

    void constructWindow() override;
};
} // namespace GUI
