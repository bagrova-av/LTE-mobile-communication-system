#pragma once
#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <optional>

template<typename T>
class ThreadSafeQueue
{
private:
    std::queue<T> queueItems;
    std::mutex queueMutex;
    std::condition_variable queueChanged;
    std::atomic<bool> isShutdown = false;

public:
    bool push(const T& item)
    {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (isShutdown)
            {
                return false;
            }
            queueItems.push(item);
        }
        queueChanged.notify_one();
        return true;
    }

    bool push(T&& item)
    {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (isShutdown)
            {
                return false;
            }
            queueItems.push(std::move(item));
        }
        queueChanged.notify_one();
        return true;
    }

    std::optional<T> pop()
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        queueChanged.wait(lock, [this]()
                                {
                                    return !queueItems.empty() || isShutdown;
                                }
        );

        if (queueItems.empty() && isShutdown)
        {
            return std::nullopt;
        }

        T item = std::move(queueItems.front());
        queueItems.pop();
        return item;
    }

    std::optional<T> tryPop()
    {
        std::lock_guard<std::mutex> lock(queueMutex);

        if (queueItems.empty())
        {
            return std::nullopt;
        }

        T item = std::move(queueItems.front());
        queueItems.pop();
        return item;
    }

    void shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            isShutdown = true;
        }
        queueChanged.notify_all();
    }

    bool isClosed() const
    {
        return isShutdown;
    }
};

#endif