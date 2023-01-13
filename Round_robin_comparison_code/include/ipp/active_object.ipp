#include "future.h"
#include "queue_entry.h"

template <typename R>
Future<R> ActiveObject::enqueue(func_type<R> func)
{
    std::unique_lock<std::mutex> lck { queue_mutex };
    auto fut = Future<R>(this);
    work_queue.push(new QueueEntry<R>(func, fut.get_promise_ptr()));
    cv.notify_one();
    return fut;
}

template <typename R>
void ActiveObject::work_until_completed(Promise<R>* p) 
{
    using namespace std::literals::chrono_literals;

    while (!p->is_complete()) {
        auto beg = std::chrono::system_clock::now();
        QEBase* qeb;
        {
            std::unique_lock lck {queue_mutex};
            while (working && work_queue.empty() && ! p->is_complete()) {
                cv.wait_for(lck, 3ms);
            }

            if (!working || p->is_complete())
                break;

            qeb = work_queue.front();
            work_queue.pop();
        }
        auto fin = std::chrono::system_clock::now();
        time += std::chrono::duration_cast<std::chrono::milliseconds>(fin-beg);
        qeb->run_and_complete();
        beg = std::chrono::system_clock::now();
        delete qeb;
        fin = std::chrono::system_clock::now();
        time += std::chrono::duration_cast<std::chrono::milliseconds>(fin-beg);
    }
}
