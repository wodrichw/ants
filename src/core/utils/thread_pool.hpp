#pragma once

#include <atomic>
#include <cassert>
#include <optional>
#include <thread>
#include <vector>
#include <deque>
#include <mutex>

using ulong = unsigned long;

template <class ThreadTask>
class ThreadWork {
    std::atomic_bool& is_working;
    std::atomic_bool const& threadpool_active;
    std::atomic_bool running_task;
    std::deque<std::unique_ptr<ThreadTask>>& pending_jobs;
    std::mutex& threadpool_mutex;
    std::optional<ThreadTask> active_task;

public:
    ThreadWork(
        std::atomic_bool& is_working,
        std::atomic_bool const& threadpool_active,
        std::deque<std::unique_ptr<ThreadTask>>& pending_jobs,
        std::mutex& threadpool_mutex
    ) :
        is_working(is_working),
        threadpool_active(threadpool_active),
        running_task(false),
        pending_jobs(pending_jobs),
        threadpool_mutex(threadpool_mutex),
        active_task()
    {}

    ThreadWork(ThreadWork const& that) :
        is_working(that.is_working),
        threadpool_active(that.threadpool_active),
        running_task(that.running_task),
        pending_jobs(that.pending_jobs),
        threadpool_mutex(threadpool_mutex),
        active_task(that.active_task)
    {}


    ThreadWork(ThreadWork&& that) :
        is_working(that.is_working),
        threadpool_active(that.threadpool_active),
        running_task(false),
        pending_jobs(that.pending_jobs),
        threadpool_mutex(that.threadpool_mutex),
        active_task(std::move(that.active_task))
    {}

    void operator()() {
        while (threadpool_active.load()) {
            {
                std::scoped_lock lock(threadpool_mutex);
                if (!pending_jobs.empty()) {
                    active_task.emplace(std::move(*pending_jobs.front()));
                    pending_jobs.pop_front();
                }
            }
            if (active_task.has_value()) {
                running_task = true;
                active_task->run();
                running_task = false;
                active_task.reset(); // Clear the active task after it's done
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
        std::deque<std::unique_ptr<ThreadTask>>& pending_jobs,
        std::mutex& threadpool_mutex
    ) :
        _is_working(false),
        _thread(ThreadWork<ThreadTask>(
            _is_working,
            threadpool_active,
            pending_jobs,
            threadpool_mutex
        ))
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
    std::vector<std::unique_ptr<ThreadWorker>> workers;
    std::deque<std::unique_ptr<ThreadTask>> pending_jobs;
    std::mutex threadpool_mutex;
    std::atomic_bool is_active;

public:
    ThreadPool(ulong number_threads) :
        workers(),
        pending_jobs(),
        threadpool_mutex(),
        is_active(true)
    {
        workers.reserve(number_threads);
        for (ulong i = 0; i < number_threads; ++i) {
            workers.push_back(std::make_unique<ThreadWorker>(is_active, pending_jobs, threadpool_mutex));
        }
    }

    // Delete the copy constructor and copy assignment operator
    ThreadPool(const ThreadPool<ThreadTask>&) = delete;
    ThreadPool& operator=(const ThreadPool<ThreadTask>&) = delete;

    void await_jobs() const {
        while (true) {
            bool all_workers_done = true;
            for (const auto& worker : workers) {
                if (worker->is_working()) {
                    all_workers_done = false;
                    break;
                }
            }
            if (all_workers_done) break;
        }
    }

    void submit_job(std::unique_ptr<ThreadTask> task) {
        std::scoped_lock lock(threadpool_mutex);
        pending_jobs.push_back(std::move(task));
    }

    ~ThreadPool() {
        is_active.store(false);
        for (auto& worker : workers) {
            worker.reset(); // This will join the thread in the worker's destructor
        }
        assert(pending_jobs.empty());
    }
};
