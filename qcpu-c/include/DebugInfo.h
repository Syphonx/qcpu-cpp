//
//	DebugInfo
//

#pragma once

#include "TokenData.h"

#include <string>
#include <unordered_map>
#include <vector>

#include <cereal/cereal.hpp>

struct DebugInfo
{
	template <class Archive>
	void serialize(Archive& archive)
	{
		archive(cereal::make_nvp("tokens", tokens));
		archive(cereal::make_nvp("labels", labels));
	}

	std::vector<TokenData> tokens;
	std::unordered_map<std::string, int32_t> labels;
};
