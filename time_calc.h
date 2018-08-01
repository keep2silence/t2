#ifndef __TIME_CALC_H__
#define __TIME_CALC_H__

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>

#define rdtsc(low,high) __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high));

static uint64_t get_cycles()
{
	uint32_t low, high;
	uint64_t val;
	rdtsc(low, high);
	val = high;
	return (val << 32) | low; 
}

static uint64_t begin_cycles, end_cycles;
#define BEGIN_CALC_CYCLES \
do {\
	begin_cycles = get_cycles (); \
} while (0)

#define END_CALC_CYCLES \
do {\
	end_cycles = get_cycles (); \
} while (0)

#define SHOW_CYCLES \
do {\
    printf ("time: %lu cycles\n", end_cycles - begin_cycles); \
} while (0)


static struct timespec tm_begin, tm_end;

#define BEGIN_CALC_TIME clock_gettime (CLOCK_MONOTONIC, &tm_begin)
#define END_CALC_TIME clock_gettime (CLOCK_MONOTONIC, &tm_end)

static long int get_nanosec ()
{
	struct timespec tm;
	clock_gettime (CLOCK_MONOTONIC, &tm);
	return tm.tv_nsec;
}

#define SHOW_TIME \
do {\
    printf ("time: %lu\n", 1000000000 * (tm_end.tv_sec - tm_begin.tv_sec) + (tm_end.tv_nsec - tm_begin.tv_nsec)); \
} while (0)

# ifdef __cplusplus
class timer_xxx 
{
public:
	timer_xxx ()
	{
		BEGIN_CALC_TIME;
	}

	~timer_xxx ()
    {
        END_CALC_TIME;
		SHOW_TIME;
    }
};

class timer_cycles 
{
public:
	timer_cycles ()
	{
		BEGIN_CALC_CYCLES;
	}

	~timer_cycles ()
    {
        END_CALC_CYCLES;
		SHOW_CYCLES;
    }
};
	
#endif

#endif
