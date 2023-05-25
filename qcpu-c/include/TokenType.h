//
//	TokenType
//

#pragma once

#include <cstdint>

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