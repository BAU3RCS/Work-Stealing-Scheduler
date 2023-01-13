#include <queue>
#include <thread>
#include <functional>
#include <stdexcept>
#include <cmath>

#include "active_object.h"
#include "scheduler.h"
#include "future.h"
#include "qentry_base.h"

using std::function;
using std::unique_lock;
using std::mutex;
using std::deque;
using std::thread;
using std::unique_lock;

// default construct everything
ActiveObject::ActiveObject(Scheduler* s)
    : scheduler{s}
{}

// assume the thread has been killed already
ActiveObject::~ActiveObject() 
{}

void ActiveObject::start() 
{
    th = thread([this]() {
        this->worker();
    });
}

void ActiveObject::shut_down() 
{
    this->enqueue<void>([this]() { this->working = false;});
}

void ActiveObject::stop() 
{
    working = false;
    cv.notify_all();
}

void ActiveObject::join() 
{
    if (th.joinable())
        th.join();
}

Scheduler* ActiveObject::get_scheduler() const {
    return scheduler;
}

std::thread::id ActiveObject::get_thread_id() const 
{
    return th.get_id();
}

void ActiveObject::worker() {
    using namespace std::chrono;
    working = true;

    while (working) {
        auto beg = std::chrono::system_clock::now();
        QEBase* qeb = nullptr;
        // deque empty state
        bool empty;
        // acquire lock check deque state
        {
            unique_lock<mutex> lck {queue_mutex};
            empty = work_queue.empty();

            // if the deque isn't  empty grab the work and do it
            if(!empty){
                qeb = work_queue.front();
                work_queue.pop_front();
            }
        }

        // if qeb is still null the deque is empty, try to steal
        if(!qeb) qeb = scheduler->steal();

        auto fin = std::chrono::system_clock::now();
        time += std::chrono::duration_cast<std::chrono::milliseconds>(fin-beg);

        // if stealing sucessful run and do task
        if(qeb){
                qeb->run_and_complete();
                delete qeb;
                beg = std::chrono::system_clock::now();
            }
        // if stealing not successful we sleep
        else{
            beg = std::chrono::system_clock::now();
            // increment failed steals
            ++failed_steals;

            // check how many times we've failed, if a lot check if we need to sleep
            // since work stealing is done at random, its a uniform chance of each worker
            // being chosen so if we fail two times the number of workers then there is most likely
            // no work to be done. I don think this will check other workers deques and lock too often
            // or at least it doesn't appear to yet
            if(failed_steals > 2*scheduler->num_workers()) scheduler->sleep();

            
            if(!sleeping){
                    // if we are supposed to be working but we failed to steal
                    // we take a chill pill for a bit
                    // I played around with the exponential growth and it appears only a 
                    // small base grows slow enough to not impact performance
                    milliseconds dur(int(pow(1.05,failed_steals)));
                    unique_lock<mutex> lck {queue_mutex};
                    cv.wait_for(lck,dur);
            }
            else{
                unique_lock<mutex> lck {queue_mutex};
                while(working && work_queue.empty() && sleeping){
                    cv.wait(lck);
                }
            }
        }

        // check if we should be working, repeat loop
        if (!working) break;
        fin = std::chrono::system_clock::now();
        time = time + std::chrono::duration_cast<std::chrono::milliseconds>(fin-beg);
    }
}

// returns from the back of the worker deque, FIFO style
QEBase* ActiveObject::steal(){
    std::unique_lock<std::mutex> lck {queue_mutex};
    if(work_queue.empty()) return nullptr;
    auto fut = work_queue.back();
    work_queue.pop_back();
    return fut;
}

// tells the worker to sleep since there is no work anywhere to be done
void ActiveObject::sleep(){
    sleeping = true;
}

// resets the worker's stealing attributes and notifies worker to check for work
void ActiveObject::wake(){
    failed_steals = 0;
    sleeping = false;
    cv.notify_all();
}

// allows the scheduler to check if the worker's deque is empty
bool ActiveObject::deque_empty(){
    std::unique_lock<std::mutex> lck {queue_mutex};
    return work_queue.empty();
}

int ActiveObject::idle_time(){
    int idle = time.count();
    std::chrono::milliseconds dur(0);
    time = dur;
    return idle;
}