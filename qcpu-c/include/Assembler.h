//
//	Assembler
//

#pragma once

#include "TokenData.h"

#include <cstdint>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>

struct Opcode
{
	Opcode(std::string name, const uint16_t value, const uint16_t arity)
		: name(std::move(name))
		, value(value)
		, arity(arity)
	{
	}

	std::string name;
	uint16_t value;
	uint16_t arity;
};

struct RegisterData
{
	RegisterData(std::string name, const uint16_t value)
		: name(std::move(name))
		, value(value)
	{
	}

	std::string name;
	uint16_t value;
};

class Assembler
{
public:
	const static std::vector<Opcode> OPS;
	const static std::vector<RegisterData> REGISTERS;

public:
	using LabelMap = std::unordered_map<std::string, int32_t>;

public:
	explicit Assembler(const std::string& file);
	
	uint16_t ParseInt(const std::string& s, uint16_t radix = 10) const;
	uint16_t ParseNumber(const std::string& s) const;
	bool IsNumber(const std::string& s) const;
	void Prepare();

	std::vector<TokenData> Tokenize();
	std::vector<uint16_t> Convert(const std::vector<TokenData>& tokens, const std::unordered_map<std::string, int32_t>& labels) const;
	std::vector<uint8_t> Assemble();
	void AssembleAndSave(const std::string& filename);
	void Load(const std::string& in);

private:
	void ReplaceText(std::string& s, const std::regex& expression, const std::string& value) const;
	bool HasMatch(const std::string& s, const std::regex& expression) const;
	std::string GetMatch(const std::string& s, const std::regex& expression, int32_t index) const;
	std::string ToLowercase(const std::string& s) const;

	std::string fileText;
	std::regex opRegex;
	std::regex registerRegex;
};
