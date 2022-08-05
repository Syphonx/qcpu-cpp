//
//	TokenType
//

#pragma once

#include <stdint.h>

enum class ETokenType : uint8_t
{
	None,
	Op,
	Register,
	ImmediateLabelReference,
	AbsoluteLabelReference,
	Immediate,
	Absolute,
	Indirect,
	Label,
	Directive
};