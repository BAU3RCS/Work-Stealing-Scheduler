#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <functional>
#include <thread>
#include <memory>
#include <vector>
#include <atomic>

#include "qentry_base.h"

template <typename R>
class Future;

template <typename R>
class Promise;

class ActiveObject;

class Scheduler {
    template <typename R>
    using func_type = std::function<R()>;

public:
    Scheduler(int num);

    template <typename R>
    Future<R> schedule(const func_type<R>& f);

    //forking for fork join work stealing scheduler
    template <typename R>
    Future<R> fork(const func_type<R>& f);

    template <typename R>
    void work_until_completed(Promise<R>* p);
    
    bool is_worker(std::thread::id tid) const;
    
    void terminate();

    QEBase* steal();

    // checks each worker deque and if empty tells each worker to sleep
    void sleep();

    // wakes worker
    void wake();

    // tells otherthings how many workers are present
    int num_workers();

    // returns total idle time of workers in milliseconds
    std::vector<double> total_idle();

private:
    std::vector<std::unique_ptr<ActiveObject>> workers;
    // number of workers
    int num;

    std::atomic<bool> sleeping {false};
};

#include "ipp/scheduler.ipp"

#endif
