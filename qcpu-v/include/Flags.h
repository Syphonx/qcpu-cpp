//
//	QCPU
//

#pragma once

#include <stdint.h>

struct Flags
{
	Flags()
		: halt(0)
		, exit(-1)
		, blok(false)
	{
	}

	int16_t halt;
	int16_t exit;
	bool blok;
};