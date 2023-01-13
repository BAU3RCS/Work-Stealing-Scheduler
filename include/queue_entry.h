#ifndef QUEUE_ENTRY_H
#define QUEUE_ENTRY_H

#include <memory>
#include <functional>

#include "qentry_base.h"
#include "future.h"

template <typename R>
class QueueEntry : public QEBase {
    using func_type = std::function<R()>;

public:
    QueueEntry(const func_type& ft, const std::shared_ptr<Promise<R>>& prom)
        : task(ft), promise(prom)
    {}

    QueueEntry(const func_type& ft) 
        : task(ft), promise{}
    {}

    virtual void run_and_complete() {
        auto val = task();
        if (promise)
            promise->complete(val);
    }

private:
    func_type task;
    std::shared_ptr<Promise<R>> promise;
};

template <>
class QueueEntry<void> : public QEBase {
    using func_type = std::function<void ()>;

public:
    QueueEntry(const func_type& ft, const std::shared_ptr<Promise<void>>& prom)
        : task(ft), promise(prom)
    {}

    QueueEntry(const func_type& ft) 
        : task(ft), promise{}
    {}

    virtual void run_and_complete() {
        task();
        if (promise)
            promise->complete();
    }

private:
    func_type task;
    std::shared_ptr<Promise<void>> promise;
};


#endif
