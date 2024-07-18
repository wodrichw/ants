#pragma once

#include <atomic>
#include <cassert>
#include <optional>
#include <thread>
#include <vector>
#include <deque>
#include <mutex>


template <class ThreadTask>
class ThreadWork { 
    bool& is_working;
    std::atomic_bool& threadpool_active;
    std::atomic_bool running_task;
    std::deque<ThreadTask>& pending_jobs;
    std::mutex& threadpool_mutex;
    std::optional<ThreadTask> active_task;

public:
    ThreadWork(
        bool& is_working,
        std::atomic_bool& threadpool_active,
        std::deque<ThreadTask>& pending_jobs,
        std::mutex& threadpool_mutex
    ):
        is_working(is_working),
        threadpool_active(threadpool_active),
        running_task(false),
        pending_jobs(pending_jobs),
        threadpool_mutex(threadpool_mutex),
        active_task()
    {}

    void operator()() {
        while( threadpool_active ) {
            {
                std::scoped_lock lock(threadpool_mutex);
                if( !pending_jobs.empty() ) {
                    active_task.emplace(std::move(pending_jobs.front()));
                    pending_jobs.pop_front();
                }
            }
            if( active_task.has_value() ) {
                running_task = true;
                active_task->run();
            }
            running_task = false;
        }
    }
};

class ThreadWorker {
    bool _is_working;
    std::thread thread;
public:
    template <class ThreadTask>
    ThreadWorker(
        std::atomic_bool& threadpool_active,
        std::deque<ThreadTask>& pending_jobs,
        std::mutex& threadpool_mutex
    ): 
        _is_working(false),
        thread(ThreadWork<ThreadTask>(_is_working, threadpool_active, pending_jobs, threadpool_mutex))
    { }

    bool is_working() const {
        return _is_working;
    }

    ~ThreadWorker() {
        thread.join();
    }
};

template <class ThreadTask>
class ThreadPool {
    std::atomic_bool is_active;
    std::vector<ThreadWorker> workers;
    std::deque<ThreadTask> pending_jobs;
    std::mutex threadpool_mutex;
public:
    ThreadPool(ulong number_threads):
        is_active(true),
        workers(),
        pending_jobs(),
        threadpool_mutex()
    {
        workers.reserve(number_threads);
        for(ulong i = 0; i < number_threads; ++i) {
            workers.push_back(ThreadWorker(is_active, pending_jobs, threadpool_mutex));
        }
    }

    void await_jobs() const {
        while( true ) {
            bool all_workers_done = true;
            for( auto& worker: workers ) {
                if( worker.is_working() ) all_workers_done = false;
            }
            if( all_workers_done ) break;
        }
    }

    void submit_job(ThreadTask task) {
        std::scoped_lock lock(threadpool_mutex);
        pending_jobs.push_back(std::move(task));
    }

    ~ThreadPool() {
        assert(pending_jobs.empty());
        is_active = false;
    }
};

