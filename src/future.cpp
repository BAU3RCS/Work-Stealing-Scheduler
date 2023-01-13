#include <thread>
#include <atomic>

#include "scheduler.h"
#include "active_object.h"
#include "future.h"

void Promise<void>::get() {
    diag("get", "starting");
    if (state < COMPLETED) {
        diag("get", "must wait");
        // wait for completion
        Scheduler* s = ao->get_scheduler();
        if (s->is_worker(std::this_thread::get_id())) {
            diag("get", "worker thread. Restarting queue");
            s->work_until_completed(this);
            diag("get", "done running queue.");
        } else {
            diag("get", "non-worker thread. sleeping.");
            std::unique_lock lck(mux);
            while (state < COMPLETED) {
                cv.wait(lck);
            }
            diag("get", "woken up");
        }
    }
    diag("get", "returning value");
    return;
}

void Promise<void>::complete() {
    int needed = NEW;
    if (state.compare_exchange_strong(needed, COMPLETING)) {
        diag("complete", "completing void");
        // hmmm...
        needed = COMPLETING;
        if (state.compare_exchange_strong(needed, COMPLETED)) {
            diag("complete", "notifying waiters");
            cv.notify_all();
        }
        else {
            throw new IllegalFutureState;
        }
    } else {
        throw new IllegalFutureState;
    }
}
