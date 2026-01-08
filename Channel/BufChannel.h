#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <stdexcept>
#include <utility>

template<typename T>
class BufChannel {
    std::queue<T> buf;
    size_t cap;
    bool stopped = false;
    std::mutex m;
    std::condition_variable cv_send;
    std::condition_variable cv_recv;

public:
    BufChannel(size_t n) : cap(n) {}

    BufChannel(const BufChannel&) = delete;
    BufChannel& operator=(const BufChannel&) = delete;

    BufChannel(BufChannel&&) = delete;
    BufChannel& operator=(BufChannel&&) = delete;

    void send(T val) {
        std::unique_lock<std::mutex> l(m);

        if (stopped) {
            throw std::runtime_error("channel is stopped");
        }

        cv_send.wait(l, [this] {
            return buf.size() < cap || stopped;
            });

        if (stopped) {
            throw std::runtime_error("channel is stopped");
        }

        buf.push(std::move(val));
        cv_recv.notify_one();
    }

    std::pair<T, bool> recv() {
        std::unique_lock<std::mutex> l(m);

        cv_recv.wait(l, [this] {
            return !buf.empty() || stopped;
            });

        if (!buf.empty()) {
            T res = std::move(buf.front());
            buf.pop();
            cv_send.notify_one();
            return { std::move(res), true };
        }

        return { T(), false };
    }

    void close() {
        std::lock_guard<std::mutex> l(m);
        stopped = true;
        cv_send.notify_all();
        cv_recv.notify_all();
    }

};
