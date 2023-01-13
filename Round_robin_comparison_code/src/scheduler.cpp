#include <memory>

#include "scheduler.h"
#include "active_object.h"

Scheduler::Scheduler(int num)
    : current{0}
{
    for (int i = 0; i < num; ++i) {
        workers.push_back(std::make_unique<ActiveObject>(this));
        workers.back()->start();
    }
}

void Scheduler::terminate() 
{
    for (auto& w : workers) {
        w->stop();
        w->join();
    }
}

bool Scheduler::is_worker(std::thread::id tid) const 
{
    for (auto& w : workers) {
        if (tid == w->get_thread_id())
            return true;
    }
    return false;
}

std::vector<double> Scheduler::total_idle(){
    std::vector<double> times;
    for (auto& w : workers) {
        times.push_back(w->idle_time());
    }
    return times;
}
