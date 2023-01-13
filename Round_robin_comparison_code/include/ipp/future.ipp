#include <thread>

#include "active_object.h"
#include "scheduler.h"

template <typename R>
R Promise<R>::get()
{
    diag("get");
    if (state < COMPLETED) {
        // wait for completion
        diag("get", "we must wait");
        Scheduler* s = ao->get_scheduler();
        if (s->is_worker(std::this_thread::get_id())) {
            diag("get", "a worker thread -- run queue");
            s->work_until_completed(this);
        }
        else {
            diag("get", "non-worker thread. Sleeping");
            std::unique_lock lck(mux);
            while (state < COMPLETED) {
                cv.wait(lck);
            }
        }
    }
    diag("get", "returning value");
    return value;
}

template <typename R>
void Promise<R>::complete(const R& v) {
    int needed = NEW;
    if (state.compare_exchange_strong(needed, COMPLETING)) {
        diag("complete", "completing value");
        this->value = v;
        needed = COMPLETING;
        if (state.compare_exchange_strong(needed, COMPLETED)) {
            diag("complete", "notifying waiters");
            cv.notify_all();
        } else {
            throw IllegalFutureState();
        }
    } else {
        throw IllegalFutureState();
    }
}

