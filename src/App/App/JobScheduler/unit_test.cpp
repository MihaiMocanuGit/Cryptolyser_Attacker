#include "JobScheduler.hpp"
#include "catch2/catch_test_macros.hpp"

#include <memory>

namespace
{

using namespace App;
class JobTest : public JobI
{
  public:
    std::atomic_flag &finish;
    std::atomic_bool &isRunning;
    size_t id;
    constexpr static std::string DESCRIPTION_PREFIX {"JobTest_"};

    JobTest(std::atomic_flag &finish, std::atomic_bool &isRunning, size_t id)
        : finish {finish}, isRunning {isRunning}, id {id}
    {
        finish.clear();
        isRunning = false;
    };
    ~JobTest() = default;

    [[nodiscard]] ExitStatus_e invoke(std::stop_token stoken) override
    {
        while (not stoken.stop_requested())
        {
            if (finish.test())
                return ExitStatus_e::OK;
        };
        return ExitStatus_e::KILLED;
    }

    [[nodiscard]] std::string description() const noexcept override
    {
        return DESCRIPTION_PREFIX + std::to_string(id);
    }

    [[nodiscard]] std::unique_ptr<JobI> clone() const override
    {
        return std::make_unique<JobTest>(*this);
    }
};

} // namespace

TEST_CASE("Empty Scheduler", "[JobScheduler]")
{
    JobScheduler scheduler {};
    REQUIRE(scheduler.state() == JobScheduler::States::NOT_STARTED);
    REQUIRE(scheduler.currentJobIndex() == JobScheduler::npos);
    REQUIRE(scheduler.jobqueueSize() == 0UL);
    REQUIRE(scheduler.allJobDescriptions().empty());

    std::atomic_flag finish = false;
    std::atomic_bool isRunning = false;
    std::unique_ptr<JobI> jobTest = std::make_unique<JobTest>(finish, isRunning, 0);

    REQUIRE(scheduler.canAddJob(jobTest));
    REQUIRE(not scheduler.canAddJob(nullptr));

    REQUIRE(scheduler.canInsertJob(scheduler.jobqueueSize(), jobTest));
    REQUIRE(not scheduler.canInsertJob(scheduler.jobqueueSize(), nullptr));
    REQUIRE(not scheduler.canInsertJob(1, jobTest));
    REQUIRE(not scheduler.canInsertJob(1, nullptr));

    REQUIRE(not scheduler.canRemoveJob(0));
    REQUIRE(not scheduler.canRemoveJob(scheduler.npos));

    REQUIRE(not scheduler.canRemoveAllJobs());

    REQUIRE(not scheduler.canUpdateJob(0, jobTest));
    REQUIRE(not scheduler.canUpdateJob(scheduler.npos, jobTest));
    REQUIRE(not scheduler.canUpdateJob(0, nullptr));
    REQUIRE(not scheduler.canUpdateJob(scheduler.npos, nullptr));

    REQUIRE(not scheduler.canCloneJob(0));
    REQUIRE(not scheduler.canCloneJob(scheduler.npos));

    REQUIRE(not scheduler.canCloneAllJobs());

    REQUIRE(not scheduler.canSwapJobs(0, 0));
    REQUIRE(not scheduler.canSwapJobs(scheduler.npos, scheduler.npos));
    REQUIRE(not scheduler.canSwapJobs(0, scheduler.npos));
    REQUIRE(not scheduler.canSwapJobs(scheduler.npos, 0));

    REQUIRE(not scheduler.canRun());
    REQUIRE(not scheduler.canPauseAfter());
    REQUIRE(not scheduler.canResume());
    REQUIRE(not scheduler.canKill());
}

TEST_CASE("Adding Jobs", "[JobScheduler]")
{
    JobScheduler scheduler {};
    constexpr unsigned JOB_COUNT {16};
    std::array<std::atomic_flag, JOB_COUNT> finishCommands {false};
    std::array<std::atomic_bool, JOB_COUNT> isRunning {false};
    for (unsigned i {0}; i < JOB_COUNT; ++i)
    {
        REQUIRE(scheduler.jobqueueSize() == i);
        std::unique_ptr<JobI> job = std::make_unique<JobTest>(finishCommands[i], isRunning[i], i);
        REQUIRE(scheduler.canAddJob(job));
        REQUIRE(scheduler.addJob(std::move(job)));
        REQUIRE(scheduler.jobqueueSize() == i + 1);

        REQUIRE(scheduler.canRemoveJob(i));
        {
            std::atomic_flag tmpFinish {false};
            std::atomic_bool tmpIsRunning {false};
            const std::unique_ptr<JobI> tmpJob =
                std::make_unique<JobTest>(tmpFinish, tmpIsRunning, scheduler.npos);
            REQUIRE(scheduler.canUpdateJob(i, tmpJob));
        }
        REQUIRE(scheduler.canCloneJob(i));
        REQUIRE(scheduler.canSwapJobs(i, i));
        REQUIRE(scheduler.canRemoveAllJobs());
        REQUIRE(scheduler.canCloneAllJobs());

        REQUIRE(scheduler.allJobDescriptions().size() == i + 1);
        REQUIRE(scheduler.cloneAllJobs().size() == i + 1);

        REQUIRE(scheduler.jobDescription(i) == JobTest::DESCRIPTION_PREFIX + std::to_string(i));

        REQUIRE(scheduler.currentJobIndex() == scheduler.npos);
        REQUIRE(scheduler.canRun());
        REQUIRE(not scheduler.canPauseAfter());
        REQUIRE(not scheduler.canResume());
        REQUIRE(not scheduler.canKill());

        REQUIRE(finishCommands[i].test() == false);
        REQUIRE(isRunning[i] == false);
    }
}

TEST_CASE("Removing Jobs", "[JobScheduler]")
{
    JobScheduler scheduler {};
    constexpr unsigned JOB_COUNT {16};
    std::array<std::atomic_flag, JOB_COUNT> finishCommands {false};
    std::array<std::atomic_bool, JOB_COUNT> isRunning {false};
    for (unsigned i {0}; i < JOB_COUNT; ++i)
    {
        std::unique_ptr<JobI> job = std::make_unique<JobTest>(finishCommands[i], isRunning[i], i);
        REQUIRE(scheduler.canAddJob(job));
        REQUIRE(scheduler.addJob(std::move(job)));
    }
    SECTION("Remove First")
    {
        for (unsigned i {0}; i < JOB_COUNT; ++i)
        {
            REQUIRE(scheduler.canRun());
            REQUIRE(scheduler.jobqueueSize() == JOB_COUNT - i);

            REQUIRE(scheduler.jobDescription(0) == JobTest::DESCRIPTION_PREFIX + std::to_string(i));

            REQUIRE(scheduler.canRemoveJob(0));
            REQUIRE(scheduler.removeJob(0));

            REQUIRE(scheduler.jobqueueSize() == JOB_COUNT - (i + 1));
        }
    }

    SECTION("Remove Last")
    {
        for (unsigned i {0}; i < JOB_COUNT; ++i)
        {
            REQUIRE(scheduler.canRun());
            REQUIRE(scheduler.jobqueueSize() == JOB_COUNT - i);

            const size_t jobIndex {scheduler.jobqueueSize() - 1};

            REQUIRE(scheduler.jobDescription(jobIndex) ==
                    JobTest::DESCRIPTION_PREFIX + std::to_string(jobIndex));

            REQUIRE(scheduler.canRemoveJob(jobIndex));
            REQUIRE(scheduler.removeJob(jobIndex));

            REQUIRE(scheduler.jobqueueSize() == JOB_COUNT - (i + 1));
        }
    }
}

TEST_CASE("Swapping Jobs", "[JobScheduler]")
{
    JobScheduler scheduler {};
    constexpr unsigned JOB_COUNT {16};
    std::array<std::atomic_flag, JOB_COUNT> finishCommands {false};
    std::array<std::atomic_bool, JOB_COUNT> isRunning {false};
    for (unsigned i {0}; i < JOB_COUNT; ++i)
    {
        std::unique_ptr<JobI> job = std::make_unique<JobTest>(finishCommands[i], isRunning[i], i);
        REQUIRE(scheduler.canAddJob(job));
        REQUIRE(scheduler.addJob(std::move(job)));
    }
    for (unsigned i {0}; 2 * i < JOB_COUNT; ++i)
    {
        REQUIRE(scheduler.canRun());
        REQUIRE(scheduler.jobqueueSize() == JOB_COUNT);

        size_t index1 {i}, index2 {JOB_COUNT - 1 - i};

        REQUIRE(scheduler.jobDescription(index1) ==
                JobTest::DESCRIPTION_PREFIX + std::to_string(index1));
        REQUIRE(scheduler.jobDescription(index2) ==
                JobTest::DESCRIPTION_PREFIX + std::to_string(index2));

        REQUIRE(scheduler.canSwapJobs(index1, index2));
        REQUIRE(scheduler.swapJobs(index1, index2));

        REQUIRE(scheduler.jobDescription(index1) ==
                JobTest::DESCRIPTION_PREFIX + std::to_string(index2));
        REQUIRE(scheduler.jobDescription(index2) ==
                JobTest::DESCRIPTION_PREFIX + std::to_string(index1));
    }
}

TEST_CASE("Updating Jobs", "[JobScheduler]")
{
    JobScheduler scheduler {};
    constexpr unsigned JOB_COUNT {16};
    std::array<std::atomic_flag, JOB_COUNT> finishCommands {false};
    std::array<std::atomic_bool, JOB_COUNT> isRunning {false};
    for (unsigned i {0}; i < JOB_COUNT; ++i)
    {
        std::unique_ptr<JobI> job = std::make_unique<JobTest>(finishCommands[i], isRunning[i], i);
        REQUIRE(scheduler.canAddJob(job));
        REQUIRE(scheduler.addJob(std::move(job)));
    }
    for (unsigned i {0}; i < JOB_COUNT; ++i)
    {
        REQUIRE(scheduler.canRun());
        REQUIRE(scheduler.jobqueueSize() == JOB_COUNT);

        REQUIRE(scheduler.jobDescription(i) == JobTest::DESCRIPTION_PREFIX + std::to_string(i));

        std::atomic_flag tmpFinish {false};
        std::atomic_bool tmpIsRunning {false};
        std::unique_ptr<JobI> tmpJob =
            std::make_unique<JobTest>(tmpFinish, tmpIsRunning, JOB_COUNT + i);
        REQUIRE(scheduler.canUpdateJob(i, tmpJob));
        REQUIRE(scheduler.updateJob(i, std::move(tmpJob)));

        REQUIRE(scheduler.jobDescription(i) ==
                JobTest::DESCRIPTION_PREFIX + std::to_string(JOB_COUNT + i));
    }
}

TEST_CASE("Running", "[JobScheduler]")
{
    JobScheduler scheduler {};
    constexpr unsigned JOB_COUNT {16};
    static_assert(JOB_COUNT > 1, "More than one job is needed");
    std::array<std::atomic_flag, JOB_COUNT> finishCommands {false};
    std::array<std::atomic_bool, JOB_COUNT> isRunning {false};
    for (unsigned i {0}; i < JOB_COUNT; ++i)
    {
        std::unique_ptr<JobI> job = std::make_unique<JobTest>(finishCommands[i], isRunning[i], i);
        REQUIRE(scheduler.canAddJob(job));
        REQUIRE(scheduler.addJob(std::move(job)));
    }
    REQUIRE(scheduler.state() == JobScheduler::States::NOT_STARTED);
    REQUIRE(scheduler.currentJobIndex() == scheduler.npos);
    REQUIRE(scheduler.run());

    SECTION("Simple")
    {
        for (unsigned i {0}; i < JOB_COUNT; ++i)
        {
            INFO(std::format("Loop: {}", i));
            REQUIRE(scheduler.state() == JobScheduler::States::RUNNING);
            REQUIRE(scheduler.currentJobIndex() == i);
            finishCommands[i].test_and_set();
            // The job might have finished, but currentJobIndex is incremented after a few checks
            // are done. Thus, the test waits a few milliseconds to let the job thread do its
            // checks.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    SECTION("Rerun")
    {
        for (unsigned i {0}; i < JOB_COUNT; ++i)
        {
            INFO(std::format("Loop: {}", i));
            REQUIRE(scheduler.state() == JobScheduler::States::RUNNING);
            REQUIRE(scheduler.currentJobIndex() == i);
            finishCommands[i].test_and_set();
            // The job might have finished, but currentJobIndex is incremented after a few checks
            // are done. Thus, the test waits a few milliseconds to let the job thread do its
            // checks.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        std::for_each(finishCommands.begin(), finishCommands.end(),
                      [](auto &flag) { flag.clear(); });

        REQUIRE(scheduler.state() == JobScheduler::States::NOT_STARTED);
        REQUIRE(scheduler.currentJobIndex() == scheduler.npos);
        REQUIRE(scheduler.run());
        for (unsigned i {0}; i < JOB_COUNT; ++i)
        {
            INFO(std::format("Loop: {}", i));
            REQUIRE(scheduler.state() == JobScheduler::States::RUNNING);
            REQUIRE(scheduler.currentJobIndex() == i);
            finishCommands[i].test_and_set();
            // The job might have finished, but currentJobIndex is incremented after a few checks
            // are done. Thus, the test waits a few milliseconds to let the job thread do its
            // checks.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    SECTION("Pause - Resume")
    {
        // The last job is not run in this loop
        for (unsigned i {0}; i + 1 < JOB_COUNT; ++i)
        {
            INFO(std::format("Loop: {}", i));
            REQUIRE(scheduler.state() == JobScheduler::States::RUNNING);
            REQUIRE(scheduler.currentJobIndex() == i);

            REQUIRE(scheduler.canPauseAfter());
            REQUIRE(scheduler.pauseAfter());
            REQUIRE(scheduler.state() == JobScheduler::States::RUNNING);

            finishCommands[i].test_and_set();
            // The job might have finished, but currentJobIndex is incremented after a few checks
            // are done. Thus, the test waits a few milliseconds to let the job thread do its
            // checks.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            REQUIRE(scheduler.state() == JobScheduler::States::PAUSED);
            REQUIRE(scheduler.currentJobIndex() == i + 1);
            REQUIRE(scheduler.canResume());
            REQUIRE(scheduler.resume());
        }
        finishCommands[JOB_COUNT - 1].test_and_set();
    }

    SECTION("Kill - Resume")
    {
        for (unsigned i {0}; i < JOB_COUNT; ++i)
        {
            INFO(std::format("Loop: {}", i));
            REQUIRE(scheduler.state() == JobScheduler::States::RUNNING);
            REQUIRE(scheduler.currentJobIndex() == i);

            REQUIRE(scheduler.canKill());
            REQUIRE(scheduler.kill());

            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            REQUIRE(scheduler.canResume());
            REQUIRE(scheduler.resume());
            finishCommands[i].test_and_set();

            // The job might have finished, but currentJobIndex is incremented after a few checks
            // are done. Thus, the test waits a few milliseconds to let the job thread do its
            // checks.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    // Even though all the jobs have finished, the state cannot be immediately updated, as this
    // happens in another thread
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    INFO("Finished all jobs");
    REQUIRE(scheduler.state() == JobScheduler::States::NOT_STARTED);
    REQUIRE(scheduler.currentJobIndex() == scheduler.npos);
}

TEST_CASE("Reordering", "[JobScheduler]")
{
    JobScheduler scheduler {};
    constexpr unsigned JOB_COUNT {16};
    static_assert(JOB_COUNT > 1, "More than one job is needed");
    std::array<std::atomic_flag, JOB_COUNT> finishCommands {false};
    std::array<std::atomic_bool, JOB_COUNT> isRunning {false};
    for (unsigned i {0}; i < JOB_COUNT; ++i)
    {
        std::unique_ptr<JobI> job = std::make_unique<JobTest>(finishCommands[i], isRunning[i], i);
        REQUIRE(scheduler.canAddJob(job));
        REQUIRE(scheduler.addJob(std::move(job)));
    }

    SECTION("While Stopped")
    {
        for (unsigned i {0}; 2 * i < JOB_COUNT; ++i)
        {
            INFO(std::format("Loop: {}", i));
            REQUIRE(scheduler.jobDescription(i) == JobTest::DESCRIPTION_PREFIX + std::to_string(i));
            REQUIRE(scheduler.canSwapJobs(i, JOB_COUNT - 1 - i));
            REQUIRE(scheduler.swapJobs(i, JOB_COUNT - 1 - i));
            REQUIRE(scheduler.jobDescription(i) ==
                    JobTest::DESCRIPTION_PREFIX + std::to_string(JOB_COUNT - 1 - i));
            finishCommands[i].test_and_set();
            // The job might have finished, but currentJobIndex is incremented after a few checks
            // are done. Thus, the test waits a few milliseconds to let the job thread do its
            // checks.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
