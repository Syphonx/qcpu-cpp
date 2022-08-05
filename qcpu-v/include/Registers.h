//
//	QCPU
//

#pragma once

#include <stdint.h>

struct Registers
{
	Registers()
		: a(0)
		, b(0)
		, c(0)
		, d(0)
		, x(0)
		, y(0)
	{
	}

	uint16_t a;
	uint16_t b;
	uint16_t c;
	uint16_t d;
	uint16_t x;
	uint16_t y;
};