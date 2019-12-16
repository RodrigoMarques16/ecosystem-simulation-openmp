#pragma once

#include <deque>
#include <condition_variable>
#include <mutex>

/**
 * Wrapper around std::vector for thread safe push_back
 */

template <typename T>
struct ConcurrentVector {
    using Container = std::vector<T>;
    using Lock      = std::unique_lock<std::mutex>;

    Container vec;
    
    std::mutex mut;
    std::condition_variable cond;

    ConcurrentVector() = default;
    ~ConcurrentVector() = default;

    void push_back(T&& item) {
        Lock lk{mut};
        vec.emplace_back(std::move(item));
        lk.unlock();
        cond.notify_one();
    }

    const T& operator[](size_t i) const {
        return vec[i];
    }

    int size() {
        Lock mlock{mut};
        return vec.size();
    }

    void clear() {
        Lock mlock{mut};
        vec.clear();
    }
};
