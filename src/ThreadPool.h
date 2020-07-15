#pragma once
#include <thread>
#include <vector>
#include <exception>

#include "utils.h"

class ThreadPool
{
public:
    template<typename F, typename C>
    ThreadPool(uint8 nr, F function, C context)
    {
        if (nr == 0)
        {
            throw std::logic_error("There are no threads in thread pool!");
        }

        _run = true;
        for (uint8 i = 0; i < nr; i++)
        {
            _threads.emplace_back(function, context, &_run);
        }
    }

    ~ThreadPool()
    {
        _run = false;
        for (auto& thread : _threads)
        {
            thread.join();
        }
    }
private:
    std::vector<std::thread> _threads;
    std::atomic<bool> _run;

};