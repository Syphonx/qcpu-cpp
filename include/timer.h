//
//	Timer
//

#pragma once

struct Timer
{
	Uint64 last;

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

	Uint64 Now()
	{
		return SDL_GetPerformanceCounter();
	}

	void Reset()
	{
		last = Now();
	}
};