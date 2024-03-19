#include "simcoe/units.hpp"

#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <iostream>

std::atomic_size_t gMaxIEC{};
std::atomic_size_t gMaxSI{};
std::string gLargestIEC{};
std::string gLargestSI{};

static std::mutex gMutex;

static void println_unlocked(auto&&... args)
{
    (std::cout << ... << args) << std::endl;
}

static void println(auto&&... args)
{
    std::lock_guard<std::mutex> lock{gMutex};
    println_unlocked(std::forward<decltype(args)>(args)...);
}

static void set_largest_iec(size_t len, const char *begin, const char *end)
{
    // initial test to not thrash the mutex
    if (len > gMaxIEC)
    {
        std::lock_guard<std::mutex> lock{gMutex};

        // retest in case another thread updated the value
        if (len > gMaxIEC)
        {
            gMaxIEC = len;
            gLargestIEC = std::string{begin, end};

            println_unlocked("New largest IEC ", len, ": ", gLargestIEC);
        }
    }
}

static void set_largest_si(size_t len, const char *begin, const char *end)
{
    // initial test to not thrash the mutex
    if (len > gMaxSI)
    {
        std::lock_guard<std::mutex> lock{gMutex};

        // retest in case another thread updated the value
        if (len > gMaxSI)
        {
            gMaxSI = len;
            gLargestSI = std::string{begin, end};

            println_unlocked("New largest SI ", len, ": ", gLargestSI);
        }
    }
}

int main()
{
    size_t count = std::thread::hardware_concurrency();
    std::vector<std::thread> workers;
    workers.reserve(count);

    // partition the work evenly
    uintmax_t fraction = UINTMAX_MAX / count;

    for (size_t i = 0; i < count; i++)
    {
        uintmax_t start = i * fraction;
        uintmax_t end = (i + 1) * fraction;

        workers.emplace_back([start, end, i] {
            char buffer[0x1000];

            println("Thread ", std::dec, i, " start ", std::hex, start, " end ", end);

            // iterate backwards to find the largest value faster
            for (uintmax_t j = end; j > start; j--)
            {
                sm::Memory mem{j};

                size_t iec_len = mem.to_chars(buffer, std::size(buffer), sm::Memory::IEC);
                set_largest_iec(iec_len, buffer, buffer + iec_len);

                size_t si_len = mem.to_chars(buffer, std::size(buffer), sm::Memory::SI);
                set_largest_si(si_len, buffer, buffer + si_len);
            }
        });
    }

    // join the threads
    for (auto &worker : workers)
    {
        worker.join();
    }

    println("Largest IEC ", gMaxIEC, ": ", gLargestIEC);
    println("Largest SI ", gMaxSI, ": ", gLargestSI);
}
