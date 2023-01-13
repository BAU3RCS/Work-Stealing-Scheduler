#include <cstdlib>

#include "active_object.h"
#include "future.h"

template <typename R>
Future<R> Scheduler::schedule(const func_type<R>& f){
    // implement random seed and rand() so that mulitple tasks
    // are distributed among multiple workers more evenly fro the get go
    // then work stealing takes over

    if(sleeping) wake();

    srand(time(nullptr));
    return workers[rand() % workers.size()]->enqueue(f);
}

template <typename R>
void Scheduler::work_until_completed(Promise<R>* p) 
{
    auto tid = std::this_thread::get_id();
    for (auto& w : workers) {
        if (tid == w->get_thread_id()) {
            w->work_until_completed(p);
            return;
        }
    }
}

template <typename R>
Future<R> Scheduler::fork(const func_type<R>& f){

    if(sleeping) wake();

    // find current thread
    auto tid = std::this_thread::get_id();
    // find worker that is current trhread
    for (auto& w : workers) {
        if (tid == w->get_thread_id()) {
            //give forked work and return future
            return w->enqueue(f);
        }
    }
    // shutup compiler, if we get here we are all doomed
    // the commies have done it, they have infiltrated us
    return nullptr;
}
