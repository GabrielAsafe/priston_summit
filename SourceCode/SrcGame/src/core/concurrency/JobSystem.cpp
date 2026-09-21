#include "core/concurrency/JobSystem.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>

namespace summit {
namespace core {
namespace concurrency {

namespace
{
thread_local bool gSummitJobWorker = false;

std::size_t ParsePositiveSize(const char *value)
{
    if (!value || !*value)
        return 0;
    char *end = nullptr;
    const unsigned long parsed = std::strtoul(value, &end, 10);
    if (!end || end == value || *end != '\0' || parsed == 0)
        return 0;
    return static_cast<std::size_t>(parsed);
}
}

JobGroup::JobGroup()
    : pending_(0)
{
}

bool JobGroup::IsComplete() const
{
    return pending_.load(std::memory_order_acquire) == 0;
}

std::uint32_t JobGroup::Pending() const
{
    return pending_.load(std::memory_order_acquire);
}

void JobGroup::Wait()
{
    JobSystem::Instance().Wait(*this);
}

void JobGroup::Add()
{
    pending_.fetch_add(1, std::memory_order_acq_rel);
}

void JobGroup::CompleteOne()
{
    if (pending_.fetch_sub(1, std::memory_order_acq_rel) == 1)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        cv_.notify_all();
    }
}

JobSystem &JobSystem::Instance()
{
    static JobSystem instance;
    return instance;
}

std::size_t JobSystem::ResolveWorkerCount()
{
    const std::size_t forced = ParsePositiveSize(std::getenv("SUMMIT_WORKER_THREADS"));
    if (forced)
        return std::min<std::size_t>(forced, 32u);

    const unsigned hardware = std::thread::hardware_concurrency();
    if (hardware <= 1u)
        return 1u;

    // Keep one logical CPU for the game/render thread. The cap protects the
    // 32-bit client from excessive thread stacks on very large hosts.
    return std::min<std::size_t>(static_cast<std::size_t>(hardware - 1u), 16u);
}

JobSystem::JobSystem()
    : stopping_(false), pendingJobs_(0), completedJobs_(0),
      highSinceNormal_(0), nonLowSinceLow_(0)
{
    const std::size_t count = ResolveWorkerCount();
    workers_.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
        workers_.emplace_back([this]() { WorkerLoop(); });
}

JobSystem::~JobSystem()
{
    Shutdown();
}

bool JobSystem::Submit(Job job, JobPriority priority)
{
    if (!job)
        return false;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        // Recheck while holding the queue mutex. Without this, Shutdown() can
        // race between the optimistic check and insertion, leaving a job in a
        // queue after every worker has already exited.
        if (stopping_.load(std::memory_order_acquire))
            return false;

        QueuedJob queued = { std::move(job), nullptr };
        switch (priority)
        {
        case JobPriority::High: high_.push_back(std::move(queued)); break;
        case JobPriority::Low: low_.push_back(std::move(queued)); break;
        default: normal_.push_back(std::move(queued)); break;
        }
        pendingJobs_.fetch_add(1, std::memory_order_release);
    }
    cv_.notify_one();
    return true;
}

bool JobSystem::Submit(JobGroup &group, Job job, JobPriority priority)
{
    if (!job)
        return false;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopping_.load(std::memory_order_acquire))
            return false;

        // Increment the group only after the job is guaranteed to be accepted.
        // Otherwise a concurrent Shutdown() could strand the group forever.
        group.Add();
        QueuedJob queued = { std::move(job), &group };
        switch (priority)
        {
        case JobPriority::High: high_.push_back(std::move(queued)); break;
        case JobPriority::Low: low_.push_back(std::move(queued)); break;
        default: normal_.push_back(std::move(queued)); break;
        }
        pendingJobs_.fetch_add(1, std::memory_order_release);
    }
    cv_.notify_one();
    return true;
}


void JobSystem::Wait(JobGroup &group)
{
    if (group.IsComplete())
        return;

    // A worker waiting for child jobs helps execute the pool instead of
    // sleeping. This prevents nested stage/model fan-out from deadlocking on
    // forced-small pools and improves utilisation on many-core machines.
    if (gSummitJobWorker)
    {
        while (!group.IsComplete())
        {
            if (!TryExecuteOne())
            {
                std::unique_lock<std::mutex> lock(group.mutex_);
                group.cv_.wait_for(lock, std::chrono::milliseconds(1), [&group]() {
                    return group.pending_.load(std::memory_order_acquire) == 0;
                });
            }
        }
        return;
    }

    std::unique_lock<std::mutex> lock(group.mutex_);
    group.cv_.wait(lock, [&group]() {
        return group.pending_.load(std::memory_order_acquire) == 0;
    });
}

void JobSystem::WaitIdle()
{
    // Session teardown uses this instead of Shutdown(): the client can leave
    // GameMode and enter it again in the same process, so worker threads must
    // remain reusable. All producers must be quiesced by the caller first.
    if (gSummitJobWorker)
        return;

    std::unique_lock<std::mutex> lock(mutex_);
    idleCv_.wait(lock, [this]() {
        return pendingJobs_.load(std::memory_order_acquire) == 0;
    });
}

std::size_t JobSystem::WorkerCount() const
{
    return workers_.size();
}

std::uint64_t JobSystem::PendingJobs() const
{
    return pendingJobs_.load(std::memory_order_relaxed);
}

std::uint64_t JobSystem::CompletedJobs() const
{
    return completedJobs_.load(std::memory_order_relaxed);
}

bool JobSystem::IsWorkerThread() const
{
    return gSummitJobWorker;
}

bool JobSystem::TryPop(QueuedJob &job)
{
    // Strict priority is attractive for latency, but it can permanently starve
    // the adjacent map while textures keep arriving every frame. Reserve a
    // small amount of bandwidth for lower classes without letting background
    // work dominate frame-critical requests.
    if (!low_.empty() && nonLowSinceLow_ >= 16u)
    {
        job = std::move(low_.front());
        low_.pop_front();
        nonLowSinceLow_ = 0u;
        return true;
    }
    if (!normal_.empty() && highSinceNormal_ >= 6u)
    {
        job = std::move(normal_.front());
        normal_.pop_front();
        highSinceNormal_ = 0u;
        ++nonLowSinceLow_;
        return true;
    }
    if (!high_.empty())
    {
        job = std::move(high_.front());
        high_.pop_front();
        ++highSinceNormal_;
        ++nonLowSinceLow_;
        return true;
    }
    if (!normal_.empty())
    {
        job = std::move(normal_.front());
        normal_.pop_front();
        highSinceNormal_ = 0u;
        ++nonLowSinceLow_;
        return true;
    }
    if (!low_.empty())
    {
        job = std::move(low_.front());
        low_.pop_front();
        nonLowSinceLow_ = 0u;
        return true;
    }
    return false;
}

void JobSystem::Execute(QueuedJob &job)
{
    try
    {
        job.callback();
    }
    catch (...)
    {
        // Legacy callers are not exception-safe. A failed background job
        // must not kill the process or strand its JobGroup.
    }

    if (job.group)
        job.group->CompleteOne();
    if (pendingJobs_.fetch_sub(1, std::memory_order_acq_rel) == 1)
        idleCv_.notify_all();
    completedJobs_.fetch_add(1, std::memory_order_relaxed);
}

bool JobSystem::TryExecuteOne()
{
    QueuedJob job;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!TryPop(job))
            return false;
    }
    Execute(job);
    return true;
}

void JobSystem::WorkerLoop()
{
    gSummitJobWorker = true;
    for (;;)
    {
        QueuedJob job;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this]() {
                return stopping_.load(std::memory_order_acquire) ||
                       !high_.empty() || !normal_.empty() || !low_.empty();
            });

            if (stopping_.load(std::memory_order_acquire) &&
                high_.empty() && normal_.empty() && low_.empty())
                break;

            if (!TryPop(job))
                continue;
        }
        Execute(job);
    }
    gSummitJobWorker = false;
}

void JobSystem::Shutdown()
{
    bool expected = false;
    if (!stopping_.compare_exchange_strong(expected, true, std::memory_order_acq_rel))
        return;

    cv_.notify_all();
    for (std::thread &worker : workers_)
        if (worker.joinable())
            worker.join();
    workers_.clear();
}

} // namespace concurrency
} // namespace core
} // namespace summit
