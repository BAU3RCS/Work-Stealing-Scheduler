#include <memory>
#include <vector>


#include "scheduler.h"
#include "active_object.h"

Scheduler::Scheduler(int num)
    : num{num} {
    for (int i = 0; i < num; ++i) {
        workers.push_back(std::make_unique<ActiveObject>(this));
    }

    // so no crashy when the workers get greedy and steal before my pockets exist
    for(int i = 0; i < num; ++i) workers[i]->start();
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

QEBase* Scheduler::steal(){ 
    return workers[rand() % workers.size()]->steal();
}

void Scheduler::sleep(){
    if(sleeping) return;

    for (auto& w : workers)
        if(!w->deque_empty()) return;

    sleeping = true;
    for (auto& w : workers) w->sleep();
}

void Scheduler::wake(){
    sleeping = false;
    for (auto& w : workers) {
        w->wake();
    }
}

int Scheduler::num_workers(){
    return num;
}

std::vector<double> Scheduler::total_idle(){
    std::vector<double> times;
    for (auto& w : workers) {
        times.push_back(w->idle_time());
    }
    return times;
}