//
//	DebugInfo
//

#pragma once

#include "TokenType.h"

#include <cstdint>
#include <string>

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>

struct TokenData
{
	TokenData()
		: type()
		, address(0)
		, line(0)
	{
	}

	TokenData(const ETokenType type, std::string data, const int32_t address, const int32_t line)
		: type(type)
		, data(std::move(data))
		, address(address)
		, line(line)
	{
	}

	template <class Archive>
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
