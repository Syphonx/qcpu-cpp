//
//	Timer
//

#pragma once

struct Timer
{
	Uint64	Last;

	Timer()
	{
		Reset();
	}

	double	ElapsedSeconds()
	{
		return	(double)(((Now() - Last) * 1000 / (double)SDL_GetPerformanceFrequency()) * 0.001);
	}

	double	ElapsedMilliSeconds()
	{
		return	(double)((Now() - Last) * 1000 / (double)SDL_GetPerformanceFrequency());
	}

	Uint64	Now()
	{
		return SDL_GetPerformanceCounter();
	}

	void	Reset()
	{
		Last = Now();
	}
};