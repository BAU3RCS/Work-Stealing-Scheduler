#ifndef ACTIVE_OBJECT_H
#define ACTIVE_OBJECT_H

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <chrono>

#include "qentry_base.h"

class Scheduler;
template <typename R>
class Future;

template <typename R>
class Promise;

class ActiveObject {
    template <typename R>
    using func_type = std::function<R()>;
 
public:
    // Create the active object, initialize the queue, start
    // the thread running.
    ActiveObject(Scheduler* s);
   
    // tell the worker thread to shut down, and join that thread
    ~ActiveObject();

    void start();

    // push a task (a function) onto our queue. This is a tricky
    // little function! It uses some very modern C++ techniques.
    template <typename R>
    Future<R> enqueue(func_type<R> func);

    // put a poison pill into the queue so that it shuts down.
    void shut_down();

    // request immediate stop.
    void stop();

    // wait for the thread to die
    void join();

    Scheduler* get_scheduler() const;
    std::thread::id get_thread_id() const;

    template <typename R>
    void work_until_completed(Promise<R>* p);

    // returns time worker spent idle and resets time
    int idle_time();

private:
    // the function that is running on the thread we "own".
    void worker();

    std::queue<QEBase*> work_queue;
    std::mutex queue_mutex;
    std::condition_variable cv;
    bool working;
    std::thread th;
    Scheduler* scheduler;
    // time not doing work
    std::chrono::milliseconds time {0};
};

#include "ipp/active_object.ipp"

#endif
