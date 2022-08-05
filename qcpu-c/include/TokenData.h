//
//	DebugInfo
//

#pragma once

#include "TokenType.h"

#include <string>
#include <stdint.h>

#include <cereal/cereal.hpp>

struct TokenData
{
public:

	TokenData()
	{
	}

	TokenData(const ETokenType type, const std::string& data, const int32_t address, const int32_t line)
		: type(type)
		, data(data)
		, address(address)
		, line(line)
	{
	}

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(cereal::make_nvp("type", type));
		archive(cereal::make_nvp("data", data));
		archive(cereal::make_nvp("address", address));
		archive(cereal::make_nvp("line", line));
	}

	ETokenType type;
	std::string data;
	int32_t address;
	int32_t line;
};