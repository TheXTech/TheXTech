#ifndef ELAPSED_TIMER_H
#define ELAPSED_TIMER_H

#include <cstdint>

struct ElapsedTimer_p;

class ElapsedTimer
{
    ElapsedTimer_p *p = nullptr;
public:
    ElapsedTimer();
    ElapsedTimer(const ElapsedTimer &et);
    ~ElapsedTimer();
    ElapsedTimer & operator=(const ElapsedTimer &et);

    void start();
    void restart();

    int     elapsed() const;
    int64_t nanoelapsed() const;
};

#endif // ELAPSED_TIMER_H
