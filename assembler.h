//
// 
//

#pragma once

#define ASSERTF_DEF_ONCE
#include "assertf.h"

#include <stdint.h>
#include <string.h>
#include <regex>

class Assembler
{
	struct Opcode
	{
		Opcode(const std::string& name, const uint16_t value, const uint16_t arity)
			: name(name)
			, value(value)
			, arity(arity)
		{
		}

		std::string					name;
		uint16_t					value;
		uint16_t					arity;
	};

	std::vector<Opcode> ops =
	{
		Opcode("nop", 0x00, 0),
		Opcode("ext", 0x01, 1),
		Opcode("sys", 0x02, 1),
		Opcode("mov", 0x03, 2),
		Opcode("jmp", 0x04, 1),
		Opcode("jeq", 0x05, 3),
		Opcode("jne", 0x06, 3),
		Opcode("jgt", 0x07, 3),
		Opcode("jge", 0x08, 3),
		Opcode("jlt", 0x09, 3),
		Opcode("jle", 0x0A, 3),
		Opcode("jsr", 0x0B, 1),
		Opcode("ret", 0x0C, 0),
		Opcode("add", 0x0D, 2),
		Opcode("sub", 0x0E, 2),
		Opcode("mul", 0x0F, 2),
		Opcode("mod", 0x10, 2),
		Opcode("and", 0x11, 2),
		Opcode("orr", 0x12, 2),
		Opcode("not", 0x13, 1),
		Opcode("xor", 0x14, 2),
		Opcode("lsl", 0x15, 2),
		Opcode("lsr", 0x16, 2),
		Opcode("psh", 0x17, 1),
		Opcode("pop", 0x18, 1)
	};

	struct RegisterData
	{
		RegisterData(const std::string& name, const uint16_t value)
			: name(name)
			, value(value)
		{
		}

		std::string					name;
		uint16_t					value;
	};

	std::vector<RegisterData> registers =
	{
		RegisterData("a", 0x00),
		RegisterData("b", 0x01),
		RegisterData("c", 0x02),
		RegisterData("d", 0x03),
		RegisterData("x", 0x04),
		RegisterData("y", 0x05)
	};

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

	struct TokenData
	{
		TokenData(const ETokenType type, const std::string& data, const int32_t address, const int32_t line)
			: type(type)
			, data(data)
			, address(address)
			, line(line)
		{
		}

		ETokenType					type;
		std::string					data;
		int32_t						address;
		int32_t						line;
	};

public:
	Assembler(const std::string& file)
	{
		Load(file);
	}

	std::regex BuildOpcodeRegex() const
	{
		std::string regex_builder = R"(^(?:)";
		for (size_t i = 0; i < ops.size(); i++)
		{
			if (i < ops.size() - 1)
			{
				regex_builder += ops[i].name + "|";
			}
			else
			{
				regex_builder += ops[i].name;
			}
		}
		regex_builder += R"()$)";
		return std::regex(regex_builder);
	}

	std::regex BuildRegisterRegex() const
	{
		std::string regex_builder = R"(^[)";
		for (const RegisterData& reg : registers)
		{
			regex_builder += reg.name;
		}
		regex_builder += R"(]$)";
		return std::regex(regex_builder);
	}

	uint16_t ParseInt(const std::string& s, uint16_t radix = 10)
	{
		return std::stoi(s, nullptr, radix);
	}

	uint16_t ParseNumber(const std::string& s)
	{
		std::smatch match;

		// Grab all digits [0-9]
		if (std::regex_search(s.begin(), s.end(), match, std::regex(R"(^\d+$)")))
		{
			return ParseInt(s);
		}
		// Grab all the hex digits
		else if (std::regex_search(s.begin(), s.end(), match, std::regex(R"(^0x[0-9a-f]+$)", std::regex_constants::icase)))
		{
			return ParseInt(match[0], 16);
		}
		// Grab all the binary digits
		else if (std::regex_search(s.begin(), s.end(), match, std::regex(R"(^0b[01]+$)", std::regex_constants::icase)))
		{
			return ParseInt(match[1], 2);
		}

		assertf(false, "Error - Unable to parse number: %s", s.c_str());
		return 0;
	}

	bool IsNumber(const std::string& s)
	{
		if (std::regex_match(s, std::regex(R"(^\d+$)")))
		{
			return true;
		}
		else if (std::regex_match(s, std::regex(R"(^0x[0-9a-f]+$)", std::regex_constants::icase)))
		{
			return true;
		}
		else if (std::regex_match(s, std::regex(R"(^0b[01]+$)", std::regex_constants::icase)))
		{
			return true;
		}
		else
		{
			return false;
		}

		return false;
	}

	void Prepare()
	{
		text = std::regex_replace(text, std::regex(R"(\r)"), "");
		op_regex = BuildOpcodeRegex();
		reg_regex = BuildRegisterRegex();
	}

	std::vector<TokenData> Tokenize()
	{
		Prepare();

		std::vector<TokenData> tokens;
		std::vector<std::string> labels;

		int32_t			linen = 0;
		int32_t			addrs = 0;
		int32_t			index = 0;
		int32_t			depth = 0;
		std::string		token = "";

		while (index < text.size())
		{
			std::string c(1, text[index]);
			if (c == ";" || c == "#")
			{
				while (!(text[index] == '\n' || text[index] == '\0'))
				{
					index++;
				}
				continue;
			}

			if (depth == 0 && c == "(")
			{
				depth++;
			}

			if (depth > 0)
			{
				token += c;
				if (c == ")")
				{
					depth--;
				}

				if (c == "\\n")
				{
					linen++;
				}
			}
			else if (regex_match(c, std::regex(R"(\s|\0)")))
			{
				if (c == "\n")
				{
					linen++;
				}

				if (!token.empty())
				{
					ETokenType type = ETokenType::None;

					if (std::regex_match(token, op_regex))
					{
						type = ETokenType::Op;
					}
					else if (HasMatch(token, reg_regex))
					{
						type = ETokenType::Register;
					}
					else if (HasMatch(token, std::regex(R"(^\+|-$)", std::regex_constants::icase)))
					{
						type = ETokenType::ImmediateLabelReference;
					}
					else if (HasMatch(token, std::regex(R"(^[a-z]\w+$)", std::regex_constants::icase)))
					{
						type = ETokenType::ImmediateLabelReference;
					}
					else if (IsNumber(token))
					{
						type = ETokenType::Immediate;
					}
					else if (HasMatch(token, std::regex(R"(^\.\w+(?:\(.*\))$)", std::regex_constants::icase)))
					{
						type = ETokenType::Directive;
					}
					else if (HasMatch(token, std::regex(R"(^\$\w+$)", std::regex_constants::icase))
							 && IsNumber(GetMatch(token, std::regex(R"(^\$(\w+)$)"), 1)))
					{
						type = ETokenType::Absolute;
					}
					else if (HasMatch(token, std::regex(R"(^\[\w*\]$)", std::regex_constants::icase))
							 && std::regex_match(GetMatch(token, std::regex(R"(^\[(\w*)\]$)", std::regex_constants::icase), 1), reg_regex))
					{
						type = ETokenType::Indirect;
					}
					else if (std::regex_match(token, std::regex(R"(^\$(:\+|-)$)", std::regex_constants::icase)))
					{
						type = ETokenType::ImmediateLabelReference;
					}
					else if (std::regex_match(token, std::regex(R"(^\$[a-z]\w+$)", std::regex_constants::icase)))
					{
						type = ETokenType::AbsoluteLabelReference;
					}
					else
					{
						std::cout << "Unrecognised Token: " << "[" << token << "] on line " << linen << std::endl;
					}

					if (type == ETokenType::Directive)
					{
						// Directives are handles by assembler
						std::regex		expr(R"(^[\.](\w+)(?:\((.*)\))$)", std::regex_constants::icase);
						std::string		directive = ToLowercase(GetMatch(token, expr, 1));
						std::string		argument = GetMatch(token, expr, 2);

						if (directive == "org")
						{
							if (IsNumber(argument))
							{
								addrs = ParseNumber(argument);
							}
							else
							{
								assertf(false, "The argument for a .org directive must be a numeric literal");
							}
						}
						else if (directive == "text")
						{
							if (std::regex_match(argument, std::regex(R"(^'.*'$)")))
							{
								std::string temp = GetMatch(argument, std::regex("^'(.*)'$"), 1);
								ReplaceText(temp, std::regex(R"(\n)"), R"(\n)");
								for (const char byte : temp)
								{
									std::string s = std::to_string((uint16_t)byte);
									tokens.emplace_back(ETokenType::Immediate, s, addrs, linen);
									addrs++;
								}
							}
							else
							{
								assertf(false, "the argument for .text directive must be a string surrounded by \'quote marks\'");
							}
						}
						else if (directive == "ds")
						{
							if (IsNumber(argument))
							{
								addrs += ParseNumber(argument);
							}
							else
							{
								assertf(false, "The argument for a .ds directive must be a numeric literal");
							}
						}
						else
						{
							assertf(false, "Unrecognised directive: %s", directive);
						}

						token.clear();
					}
					else
					{
						tokens.emplace_back(type, token, addrs, linen);
						token.clear();
						addrs++;
					}
				}
			}
			else if (regex_match(c, std::regex(":")))
			{
				tokens.emplace_back(ETokenType::Label, token, addrs, linen);
				token.clear();
			}
			else
			{
				token += c; // still building a token...
			}

			index++;
		}

		return tokens;
	}

	std::unordered_map<std::string, int32_t> BuildLabelTable(const std::vector<TokenData>& tokens)
	{
		std::unordered_map<std::string, int32_t> table;

		for (const auto& token : tokens)
		{
			if (token.type == ETokenType::Label && !(token.data == "+" || token.data == "-"))
			{
				table[token.data] = token.address;
			}
		}

		return table;
	}

	std::vector<uint16_t> Convert(const std::vector<TokenData>& tokens, const std::unordered_map<std::string, int32_t>& labels)
	{
		std::unordered_map<ETokenType, uint16_t> addressingModeMap;
		addressingModeMap.emplace(ETokenType::Immediate, 0b00);
		addressingModeMap.emplace(ETokenType::ImmediateLabelReference, 0b00);
		addressingModeMap.emplace(ETokenType::Absolute, 0b01);
		addressingModeMap.emplace(ETokenType::AbsoluteLabelReference, 0b01);
		addressingModeMap.emplace(ETokenType::Indirect, 0b10);
		addressingModeMap.emplace(ETokenType::Register, 0b11);

		uint16_t maxAddress = 0;
		for (const auto& token : tokens)
		{
			maxAddress = (token.address > maxAddress) ? token.address : maxAddress;
		}

		std::vector<uint16_t> memory;
		memory.resize(maxAddress + 1);

		for (size_t i = 0; i < tokens.size(); i++)
		{
			const TokenData& token = tokens[i];
			uint16_t word = 0;

			switch (token.type)
			{
				case ETokenType::Op:
				{
					const Opcode& op = FindOpByName(token.data);
					std::vector<TokenData> args = std::vector<TokenData>(tokens.begin() + i + 1, tokens.begin() + i + 1 + op.arity);
					std::vector<uint16_t> types;

					for (const TokenData& arg : args)
					{
						types.push_back(addressingModeMap[arg.type]);
					}

					while (types.size() < 4)
					{
						types.push_back(0b00);
					}

					word = types[0] << 14 | types[1] << 12 | types[2] << 10 | types[3] << 8 | op.value;
				}
				break;

				case ETokenType::Register:
				{
					word = FindRegByName(token.data).value;
				}
				break;

				case ETokenType::ImmediateLabelReference:	// Fallthrough intentional
				case ETokenType::AbsoluteLabelReference:
				{
					if (token.data == "-")
					{
						for (size_t j = i - 1; j >= 0; j--)
						{
							const TokenData& t = tokens[j];
							if (t.type == ETokenType::Label && t.data == "-")
							{
								word = t.address;
								break;
							}
						}
					}
					else if (token.data == "+")
					{
						for (size_t j = i + 1; j < tokens.size(); j++)
						{
							const TokenData& t = tokens[j];
							if (t.type == ETokenType::Label && t.data == "+")
							{
								word = t.address;
								break;
							}
						}
					}
					else
					{
						std::string label = token.data;
						ReplaceText(label, std::regex(R"(^\$)"), "");
						if (labels.find(label) != labels.end())
						{
							word = labels.at(label);
						}
						else
						{
							assertf(false, "Couldn't find label: %s", label.c_str());
						}
					}

				}
				break;

				case ETokenType::Immediate:
				{
					word = ParseNumber(token.data);
				}
				break;

				case ETokenType::Absolute:
				{
					word = ParseNumber(GetMatch(token.data, std::regex(R"(\$(\w+)"), 1));
				}
				break;

				case ETokenType::Indirect:
				{
					word = FindRegByName(ToLowercase(GetMatch(token.data, std::regex(R"(\[(a|b|c|d|x|y)\])", std::regex_constants::icase), 1))).value;
				}
				break;
			}

			memory[token.address] = word;
		}

		return memory;
	}

	std::vector<uint8_t> Write(const std::vector<uint16_t>& converted)
	{
		std::vector<uint8_t> buffer;
		buffer.resize(2 * converted.size());

		for (size_t i = 0; i < converted.size(); i++)
		{
			uint16_t n = converted[i];
			uint8_t high = static_cast<uint8_t>(((0xFF00 & n) >> 8));
			uint8_t low = static_cast<uint8_t>(0x00FF & n);
			buffer[i * 2] = low;
			buffer[(i * 2) + 1] = high;
		}

		return buffer;
	}

	std::vector<uint8_t> Assemble()
	{
		auto tokens = Tokenize();
		auto labelTable = BuildLabelTable(tokens);
		auto converted = Convert(tokens, labelTable);
		auto bytes = Write(converted);

		return bytes;
	}

	void Load(const std::string& in)
	{
		std::ifstream file(in, std::ios::binary);
		file.unsetf(std::ios::skipws);

		std::streampos fileSize;
		file.seekg(0, std::ios::end);
		fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
		if (fileSize <= 0)
		{
			return;
		}

		text.reserve(fileSize);
		text.insert(text.begin(),
					std::istream_iterator<char>(file),
					std::istream_iterator<char>());
	}

private:

	Opcode& FindOpByName(const std::string& name)
	{
		for (auto& op : ops)
		{
			if (op.name == name)
			{
				return op;
			}
		}

		assertf(false, "Failed to find opcode with name: %s", name.c_str());
		return ops[0];	// Invalid
	}

	RegisterData& FindRegByName(const std::string& name)
	{
		for (auto& reg : registers)
		{
			if (reg.name == name)
			{
				return reg;
			}
		}

		assertf(false, "Failed to find register with name: %s", name.c_str());
		return registers[0];	// Invalid
	}

	void ReplaceText(std::string& s, const std::regex& expression, const std::string& value) const
	{
		s = std::regex_replace(s, expression, value);
	}

	bool HasMatch(const std::string& s, const std::regex& expression) const
	{
		return std::regex_match(s, expression);
	}

	std::string GetMatch(const std::string& s, const std::regex& expression, const int32_t index) const
	{
		std::smatch match;
		if (std::regex_search(s.begin(), s.end(), match, expression))
		{
			return match[index];
		}

		assertf(false, "Failed to find match for: %s at index %d", s.c_str(), index);
		return "";
	}

	std::string ToLowercase(const std::string& s)
	{
		std::string s2 = s;
		std::transform(s2.begin(), s2.end(), s2.begin(), tolower);
		return std::move(s2);
	}

	std::string text;
	std::regex op_regex;
	std::regex reg_regex;
};