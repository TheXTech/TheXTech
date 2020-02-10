#include "elapsed_timer.h"

#include <chrono>
#include <assert.h>

struct ElapsedTimer_p
{
    typedef std::chrono::nanoseconds TimeT;
    std::chrono::high_resolution_clock::time_point recent;
};


ElapsedTimer::ElapsedTimer()
{
    p = new ElapsedTimer_p;
    assert(p);
}

ElapsedTimer::ElapsedTimer(const ElapsedTimer &et)
{
    p = new ElapsedTimer_p;
    assert(p);
    p->recent = et.p->recent;
}

ElapsedTimer::~ElapsedTimer()
{
    assert(p);
    delete p;
}

void ElapsedTimer::start()
{
    assert(p);
    p->recent = std::chrono::high_resolution_clock::now();
}

void ElapsedTimer::restart()
{
    assert(p);
    p->recent = std::chrono::high_resolution_clock::now();
}

int ElapsedTimer::elapsed()
{
    assert(p);
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;
    return static_cast<int>(duration_cast<milliseconds>(std::chrono::high_resolution_clock::now() - p->recent).count());
}

int64_t ElapsedTimer::nanoelapsed()
{
    assert(p);
    using std::chrono::nanoseconds;
    using std::chrono::duration_cast;
    return duration_cast<nanoseconds>(std::chrono::high_resolution_clock::now() - p->recent).count();
}
