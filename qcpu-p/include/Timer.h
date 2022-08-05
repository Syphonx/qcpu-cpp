//
//	Timer
//

#pragma once

struct Timer
{
	uint64_t last;

	Timer()
	{
		Reset();
	}

	double ElapsedSeconds()
	{
		return	(double)(((Now() - last) * 1000 / (double)SDL_GetPerformanceFrequency()) * 0.001);
	}

	double ElapsedMilliSeconds()
	{
		return	(double)((Now() - last) * 1000 / (double)SDL_GetPerformanceFrequency());
	}

	uint64_t Now()
	{
		return SDL_GetPerformanceCounter();
	}

	void Reset()
	{
		last = Now();
	}
};