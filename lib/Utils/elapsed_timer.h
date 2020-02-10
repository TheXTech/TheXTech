#ifndef ELAPSED_TIMER_H
#define ELAPSED_TIMER_H

#include <stdint.h>

struct ElapsedTimer_p;

class ElapsedTimer
{
    ElapsedTimer_p *p = nullptr;
public:
    ElapsedTimer();
    ElapsedTimer(const ElapsedTimer &et);
    ~ElapsedTimer();

    void start();
    void restart();

    int     elapsed();
    int64_t nanoelapsed();
};

#endif // ELAPSED_TIMER_H
