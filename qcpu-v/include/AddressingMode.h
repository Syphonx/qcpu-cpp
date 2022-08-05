//
//	QCPU
//

#pragma once

enum class EAddressingMode
{
	Imm = 0b00,
	Abs = 0b01,
	Ind = 0b10,
	Reg = 0b11
};