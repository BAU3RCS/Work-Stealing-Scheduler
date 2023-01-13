#ifndef FUTURE_H
#define FUTURE_H

#include <mutex>
#include <condition_variable>
#include <memory>
#include <iostream>
#include <string>

class ActiveObject;
class Scheduler;

enum FutureState {
    NEW,
    COMPLETING,
    COMPLETED
};

static int root_id = 0;

constexpr bool DEBUG = false;

class IllegalFutureState { };

template <typename R>
class Promise {
public:
    Promise(ActiveObject* a) 
        : id{root_id++}, value{}, ao{a}, mux{}
    {
        diag("constructor");
    }

    bool is_complete() const {
        return state == COMPLETED;
    }

    R get();
    void complete(const R& v);
   
private:
    void diag(const std::string& func, const std::string& msg = "")
    {
        if (DEBUG) {
            std::cout << id << ": promise::" << func
                      << ", state = " << state << std::endl;
            if (msg != "") {
                std::cout << "\t-->" << msg << std::endl;
            }
        }
    }

    int id;
    R value;
    ActiveObject* ao;
    std::atomic<int> state;
    mutable std::mutex mux;
    mutable std::condition_variable cv;
};

template <>
class Promise<void> {
public:
    Promise(ActiveObject* a) 
        : id{root_id++}, ao{a}, mux{}
    {
        diag("constructor");
    }

    bool is_complete() const {
        return state == COMPLETED;
    }

    void get();
    void complete();
   
private:
    void diag(const std::string& func, const std::string& msg = "")
    {
        if (DEBUG) {
            std::cout << id << ": promise::" << func
                      << ", state = " << state << std::endl;
            if (msg != "") {
                std::cout << "\t-->" << msg << std::endl;
            }
        }
    }

    int id;
    ActiveObject* ao;
    std::atomic<int> state;
    mutable std::mutex mux;
    mutable std::condition_variable cv;
};


// T must be default-constructable and assignable.
template<typename R>
class Future {
public:
    Future(ActiveObject* a)
        : promise {std::make_shared<Promise<R>>(a)}
    {}

    bool is_complete() const {
        return promise->is_complete();
    }

    R get() {
        return promise->get();
    }

    std::shared_ptr<Promise<R>> get_promise_ptr() {
        return promise;
    }

private:
    std::shared_ptr<Promise<R>> promise;
};

#include "ipp/future.ipp"

#endif

