//
//	Assembler
//

#pragma once

#include "TokenData.h"
#include "TokenType.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdint.h>
#include <string.h>
#include <string>
#include <unordered_map>
#include <vector>

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>

struct Opcode
{
public:

	Opcode(const std::string& name, const uint16_t value, const uint16_t arity)
		: name(name)
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
public:

	RegisterData(const std::string& name, const uint16_t value)
		: name(name)
		, value(value)
	{
	}

	std::string name;
	uint16_t value;
};

class Assembler
{
public:

	const static std::vector<Opcode> ops;
	const static std::vector<RegisterData> registers;

public:

	using LabelMap = std::unordered_map<std::string, int32_t>;

public:

	Assembler(const std::string& file);

	std::regex BuildOpcodeRegex() const;
	std::regex BuildRegisterRegex() const;
	uint16_t ParseInt(const std::string& s, uint16_t radix = 10);
	uint16_t ParseNumber(const std::string& s);
	bool IsNumber(const std::string& s);
	void Prepare();

	std::vector<TokenData> Tokenize();
	LabelMap BuildLabelTable(const std::vector<TokenData>& tokens);
	std::vector<uint16_t> Convert(const std::vector<TokenData>& tokens, const std::unordered_map<std::string, int32_t>& labels);
	std::vector<uint8_t> Write(const std::vector<uint16_t>& converted);
	std::vector<uint8_t> Assemble();
	void AssembleAndSave(const std::string& filename);
	void Load(const std::string& in);

private:

	const Opcode& FindOpByName(const std::string& name) const;
	const RegisterData& FindRegByName(const std::string& name) const;
	void ReplaceText(std::string& s, const std::regex& expression, const std::string& value) const;
	bool HasMatch(const std::string& s, const std::regex& expression) const;
	std::string GetMatch(const std::string& s, const std::regex& expression, const int32_t index) const;
	std::string ToLowercase(const std::string& s);

	std::string fileText;
	std::regex opRegex;
	std::regex registerRegex;
};