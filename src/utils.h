#pragma once
//#include <stdint.h>
#include <mutex>


#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;


typedef uint32_t uint32;
typedef int32_t int32;
typedef uint16_t uint16;
typedef uint8_t uint8;
typedef uint64_t uint64;

typedef std::unique_lock<std::mutex> Guard;

class Semaphore
{
public:
    Semaphore() = default;
    Semaphore(Semaphore const& other) = delete;
protected:
    std::atomic<uint32> _sem_value = 0;
    std::mutex _sem_lock;
    std::condition_variable _sem_condition;
};

