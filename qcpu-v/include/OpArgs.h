//
//	QCPU
//

#pragma once

#include "AddressingMode.h"

#include <stdint.h>

struct OpArgs
{
	OpArgs(const uint16_t value, const EAddressingMode mode)
		: value(value)
		, mode(mode)
	{
	}

	uint16_t value;
	EAddressingMode mode;
};