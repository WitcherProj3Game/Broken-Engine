#ifndef __PERF_TIMER_H__
#define __PERF_TIMER_H__

#include "SDL/include/SDL_timer.h"
#include "BrokenCore.h"

BE_BEGIN_NAMESPACE
class BROKEN_API PerfTimer {
public:

	PerfTimer();

	void Start();

	double ReadMs() const;
	Uint64 ReadTicks() const;

private:

	Uint64 started_at;
	static Uint64 frequency;
};
BE_END_NAMESPACE
#endif