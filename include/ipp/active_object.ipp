#include "future.h"
#include "queue_entry.h"

template <typename R>
Future<R> ActiveObject::enqueue(func_type<R> func){
    std::unique_lock<std::mutex> lck { queue_mutex };
    auto fut = Future<R>(this);
    work_queue.push_front(new QueueEntry<R>(func, fut.get_promise_ptr()));
    cv.notify_one();
    return fut;
}

template <typename R>
void ActiveObject::work_until_completed(Promise<R>* p) {
    using std::unique_lock;
    using std::mutex;

    while (!p->is_complete()) {
        auto beg = std::chrono::system_clock::now();
        QEBase* qeb = nullptr;
        bool empty;

        // acquire lock and check deque
        {
            unique_lock<mutex> lck {queue_mutex};
            empty = work_queue.empty();

            // work if there is something
            if(!empty){
                qeb = work_queue.front();
                work_queue.pop_front();
            }
        }

        // if nothing steal
        if(!qeb) qeb = scheduler->steal();

        auto fin = std::chrono::system_clock::now();
        time += std::chrono::duration_cast<std::chrono::milliseconds>(fin-beg);

        // if a task was stolen do it!
        if(qeb){
                qeb->run_and_complete();
                delete qeb;
            }
        beg = std::chrono::system_clock::now();
        // never sleep because there should be work somewhere to steal

        // check if our other work we are waiting on is done or we need to stop we exit
        if (!working || p->is_complete()) break;

        fin = std::chrono::system_clock::now();
        time += std::chrono::duration_cast<std::chrono::milliseconds>(fin-beg);
    }
}