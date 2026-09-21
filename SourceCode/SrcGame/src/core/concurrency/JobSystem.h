#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace summit {
namespace core {
namespace concurrency {

enum class JobPriority : std::uint8_t
{
    High = 0,
    Normal = 1,
    Low = 2
};

class JobGroup
{
public:
    JobGroup();
    JobGroup(const JobGroup &) = delete;
    JobGroup &operator=(const JobGroup &) = delete;

    bool IsComplete() const;
    std::uint32_t Pending() const;
    void Wait();

private:
    friend class JobSystem;
    void Add();
    void CompleteOne();

    std::atomic<std::uint32_t> pending_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
};

class JobSystem
{
public:
    using Job = std::function<void()>;

    static JobSystem &Instance();

    JobSystem(const JobSystem &) = delete;
    JobSystem &operator=(const JobSystem &) = delete;

    bool Submit(Job job, JobPriority priority = JobPriority::Normal);
    bool Submit(JobGroup &group, Job job, JobPriority priority = JobPriority::Normal);
    void Wait(JobGroup &group);
    void WaitIdle();

    std::size_t WorkerCount() const;
    std::uint64_t PendingJobs() const;
    std::uint64_t CompletedJobs() const;

    // Worker-side waits are cooperative: the waiting worker helps execute
    // queued jobs, so nested fan-out remains safe even with a one-worker pool.
    bool IsWorkerThread() const;

    void Shutdown();

private:
    struct QueuedJob
    {
        Job callback;
        JobGroup *group;
    };

    JobSystem();
    ~JobSystem();

    static std::size_t ResolveWorkerCount();
    bool TryPop(QueuedJob &job);
    bool TryExecuteOne();
    void Execute(QueuedJob &job);
    void WorkerLoop();

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::condition_variable idleCv_;
    std::deque<QueuedJob> high_;
    std::deque<QueuedJob> normal_;
    std::deque<QueuedJob> low_;
    std::vector<std::thread> workers_;
    std::atomic<bool> stopping_;
    std::atomic<std::uint64_t> pendingJobs_;
    std::atomic<std::uint64_t> completedJobs_;
    // Protected by mutex_. Prevents continuous frame-critical work from
    // starving model work and speculative adjacent-stage streaming forever.
    std::uint32_t highSinceNormal_;
    std::uint32_t nonLowSinceLow_;
};

} // namespace concurrency
} // namespace core
} // namespace summit
