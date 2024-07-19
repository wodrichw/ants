#pragma once

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <optional>
#include <thread>
#include <list>
#include <deque>
#include <mutex>

#include "app/globals.hpp"
#include "spdlog/spdlog.h"

using ulong = unsigned long;

template <class ThreadTask>
class ThreadWork {
    std::atomic_bool& is_working;
    std::atomic_bool const& threadpool_active;
    std::atomic_bool running_task;
    std::deque<ThreadTask>& pending_jobs;
    std::optional<ThreadTask> active_task;
    // TODO: consolidate counts into one for just the number of unfinished jobs
    std::atomic_ullong& pending_jobs_count;
    std::atomic_ullong& active_worker_count;

public:
    explicit ThreadWork(
        std::atomic_bool& is_working,
        std::atomic_bool const& threadpool_active,
        std::deque<ThreadTask>& pending_jobs,
        std::atomic_ullong& pending_jobs_count,
        std::atomic_ullong& active_worker_count
    ) :
        is_working(is_working),
        threadpool_active(threadpool_active),
        running_task(false),
        pending_jobs(pending_jobs),
        active_task(),
        pending_jobs_count(pending_jobs_count),
        active_worker_count(active_worker_count)
    {}

    void operator()() {
        std::chrono::nanoseconds initial_backoff(1000);
        std::chrono::nanoseconds backoff(initial_backoff);
        while ( threadpool_active ) {
            {
                std::scoped_lock lock(globals::threadpool_mutex);
                if (!pending_jobs.empty()) {
                    active_task.emplace(std::move(pending_jobs.front()));
                    pending_jobs.pop_front();
                    ++active_worker_count;
                    --pending_jobs_count;
                    SPDLOG_TRACE("aquired async op");
                    backoff = initial_backoff;
                    
                } 
            }
            if (active_task.has_value()) {
                running_task = true;
                active_task->run();
                --active_worker_count;
                running_task = false;
                active_task.reset(); // Clear the active task after it's done
            } else {
                std::this_thread::__sleep_for(std::chrono::seconds(0), backoff);
                backoff *= 2;
            }
        }
    }
};

class ThreadWorker {
    std::atomic_bool _is_working;
    std::thread _thread;
public:
    template <class ThreadTask>
    ThreadWorker(
        std::atomic_bool const& threadpool_active,
        std::deque<ThreadTask>& pending_jobs,
        std::atomic_ullong& pending_jobs_count,
        std::atomic_ullong& active_worker_count
    ) :
        _is_working(false),
        _thread([&]() {
            ThreadWork<ThreadTask> thread_work(
                _is_working,
                threadpool_active,
                pending_jobs,
                pending_jobs_count,
                active_worker_count
            );
            thread_work();
        })
    { }

    bool is_working() const {
        return _is_working;
    }

    ~ThreadWorker() {
        if (_thread.joinable()) {
            _thread.join();
        }
    }

    // Delete the copy constructor and copy assignment operator
    ThreadWorker(const ThreadWorker&) = delete;
    ThreadWorker& operator=(const ThreadWorker&) = delete;
};

template <class ThreadTask>
class ThreadPool {
    std::list<ThreadWorker> workers;
    std::deque<ThreadTask> pending_jobs;
    std::atomic_bool is_active;
    std::atomic_ullong pending_jobs_count;
    std::atomic_ullong active_worker_count;

public:
    ThreadPool(ulong number_threads) :
        workers(),
        pending_jobs(),
        is_active(true),
        pending_jobs_count(),
        active_worker_count()
    {
        for (ulong i = 0; i < number_threads; ++i) {
            workers.emplace_back(is_active, pending_jobs, pending_jobs_count, active_worker_count);
        }
    }

    // Delete the copy constructor and copy assignment operator
    ThreadPool(const ThreadPool<ThreadTask>&) = delete;
    ThreadPool& operator=(const ThreadPool<ThreadTask>&) = delete;

    void await_jobs() const {
        while (active_worker_count > 0 || pending_jobs_count > 0) {
            /*do nothing */
        }
    }

    void submit_job(ThreadTask& task) {
        ++pending_jobs_count;
        std::scoped_lock lock(globals::threadpool_mutex);
        pending_jobs.push_back(std::move(task));
    }

    ~ThreadPool() {
        is_active = false;
        assert(pending_jobs.empty());
    }
};



