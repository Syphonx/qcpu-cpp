//
//	Assembler
//

#include "Assembler.h"
#include "DebugInfo.h"
#include "Filesystem.h"

#define ASSERTF_DEF_ONCE
#include "assertf.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <cereal/archives/json.hpp>

const std::vector<Opcode> Assembler::OPS =
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

const std::vector<RegisterData> Assembler::REGISTERS =
{
	RegisterData("a", 0x00),
	RegisterData("b", 0x01),
	RegisterData("c", 0x02),
	RegisterData("d", 0x03),
	RegisterData("x", 0x04),
	RegisterData("y", 0x05)
};

namespace AssemblerPrivate
{
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

	const Opcode& FindOpByName(const std::string& name)
	{
		for (auto& op : Assembler::OPS)
		{
			if (op.name == name)
			{
				return op;
			}
		}

		assertf(false, "Failed to find opcode with name: %s", name.c_str());
		return Assembler::OPS[0]; // Invalid
	}

	const RegisterData& FindRegByName(const std::string& name)
	{
		for (auto& reg : Assembler::REGISTERS)
		{
			if (reg.name == name)
			{
				return reg;
			}
		}

		assertf(false, "Failed to find register with name: %s", name.c_str());
		return Assembler::REGISTERS[0]; // Invalid
	}
}

Assembler::Assembler(const std::string& file)
	: fileText()
	, opRegex()
	, registerRegex()
{
	Load(file);
}

uint16_t Assembler::ParseInt(const std::string& s, uint16_t radix /*= 10*/) const
{
	return std::stoi(s, nullptr, radix);
}

uint16_t Assembler::ParseNumber(const std::string& s) const
{
	std::smatch match;

	// Grab all digits [0-9]
	if (std::regex_search(s.begin(), s.end(), match, std::regex(R"(^\d+$)")))
	{
		return ParseInt(s);
	}
	// Grab all the hex digits
	if (std::regex_search(s.begin(), s.end(), match, std::regex(R"(^0x([0-9a-f]+)$)", std::regex_constants::icase)))
	{
		return ParseInt(match[0], 16);
	}
	// Grab all the binary digits
	if (std::regex_search(s.begin(), s.end(), match, std::regex(R"(^0b([01]+)$)", std::regex_constants::icase)))
	{
		return ParseInt(match[1], 2);
	}

	assertf(false, "Error - Unable to parse number: %s", s.c_str());
	return 0;
}

bool Assembler::IsNumber(const std::string& s) const
{
	if (std::regex_match(s, std::regex(R"(^\d+$)")))
	{
		return true;
	}
	if (std::regex_match(s, std::regex(R"(^0x[0-9a-f]+$)", std::regex_constants::icase)))
	{
		return true;
	}
	if (std::regex_match(s, std::regex(R"(^0b[01]+$)", std::regex_constants::icase)))
	{
		return true;
	}
	return false;

	return false;
}

void Assembler::Prepare()
{
	auto buildOpcodeRegex = []
	{
		std::string regexBuilder = R"(^(?:)";
		for (size_t i = 0; i < OPS.size(); i++)
		{
			if (i < OPS.size() - 1)
			{
				regexBuilder += OPS[i].name + "|";
			}
			else
			{
				regexBuilder += OPS[i].name;
			}
		}
		regexBuilder += R"()$)";
		return std::regex(regexBuilder);
	};

	auto buildRegisterRegex = []
	{
		std::string regexBuilder = R"(^[)";
		for (const RegisterData& reg : REGISTERS)
		{
			regexBuilder += reg.name;
		}
		regexBuilder += R"(]$)";
		return std::regex(regexBuilder);
	};

	fileText = std::regex_replace(fileText, std::regex(R"(\r)"), "");
	opRegex = buildOpcodeRegex();
	registerRegex = buildRegisterRegex();
}

std::vector<TokenData> Assembler::Tokenize()
{
	Prepare();

	std::vector<TokenData> tokens;
	std::vector<std::string> labels;

	int32_t line = 1;
	int32_t address = 0;
	int32_t depth = 0;
	int32_t index = 0;
	int32_t size = static_cast<int32_t>(fileText.size());
	std::string token;

	while (index <= size)
	{
		// Grab the character at [index]
		std::string c(1, fileText[index]);

		// If it's a ';' or a '#', consume until we hit a new line or the end of the file 
		if (c == ";" || c == "#")
		{
			while (!(fileText[index] == '\n' || fileText[index] == '\0'))
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
				line++;
			}
		}
		else
		{
			// Are we whitespace or at the end of the file?
			if (regex_match(c, std::regex(R"(\s|\0)")))
			{
				if (!token.empty())
				{
					auto type = ETokenType::None;

					if (std::regex_match(token, opRegex))
					{
						type = ETokenType::Op;
					}
					else if (HasMatch(token, registerRegex))
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
						&& std::regex_match(GetMatch(token, std::regex(R"(^\[(\w*)\]$)", std::regex_constants::icase), 1), registerRegex))
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
						std::cout << "Unrecognised Token: " << "[" << token << "] on line " << line << std::endl;
					}

					if (type == ETokenType::Directive)
					{
						// Directives are handles by assembler
						std::regex expr(R"(^[\.](\w+)(?:\((.*)\))$)", std::regex_constants::icase);
						std::string directive = ToLowercase(GetMatch(token, expr, 1));
						std::string argument = GetMatch(token, expr, 2);

						if (directive == "org")
						{
							if (IsNumber(argument))
							{
								address = ParseNumber(argument);
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
									std::string s = std::to_string(static_cast<uint16_t>(byte));
									tokens.emplace_back(ETokenType::Immediate, s, address, line);
									address++;
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
								address += ParseNumber(argument);
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
						tokens.emplace_back(type, token, address, line);
						token.clear();
						address++;
					}
				}

				if (c == "\n")
				{
					line++;
				}
			}
			else
			{
				// Are we build a label?
				if (regex_match(c, std::regex(":")))
				{
					tokens.emplace_back(ETokenType::Label, token, address, line);
					token.clear();
				}
				else
				{
					token += c; // still building a token...
				}
			}
		}

		index++;
	}

	return tokens;
}

std::vector<uint16_t> Assembler::Convert(const std::vector<TokenData>& tokens, const std::unordered_map<std::string, int32_t>& labels) const
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
				const Opcode& op = AssemblerPrivate::FindOpByName(token.data);
				auto args = std::vector<TokenData>(tokens.begin() + i + 1, tokens.begin() + i + 1 + op.arity);
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
				word = AssemblerPrivate::FindRegByName(token.data).value;
			}
			break;

			case ETokenType::ImmediateLabelReference: // Fallthrough intentional
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
				word = AssemblerPrivate::FindRegByName(ToLowercase(GetMatch(token.data, std::regex(R"(\[(a|b|c|d|x|y)\])", std::regex_constants::icase), 1))).value;
			}
			break;

			default:
			{
			}
			break;
		}

		memory[token.address] = word;
	}

	return memory;
}

std::vector<uint8_t> Assembler::Assemble()
{
	auto tokens = Tokenize();
	auto labelTable = AssemblerPrivate::BuildLabelTable(tokens);
	auto converted = Convert(tokens, labelTable);
	auto bytes = AssemblerPrivate::Write(converted);

	return bytes;
}

void Assembler::AssembleAndSave(const std::string& filename)
{
	auto tokens = Tokenize();
	auto labelTable = AssemblerPrivate::BuildLabelTable(tokens);
	auto converted = Convert(tokens, labelTable);
	auto bytes = AssemblerPrivate::Write(converted);

	std::ofstream file(filename, std::ios::out | std::ios::binary);
	if (!bytes.empty())
	{
		file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
	}

	FileWriter writer;
	if (writer.Open(filename + ".debug"))
	{
		DebugInfo debug;
		cereal::JSONOutputArchive archive(writer.GetStream());
		debug.tokens = tokens;
		debug.labels = labelTable;
		archive(cereal::make_nvp("debug", debug));
	}
}

void Assembler::Load(const std::string& in)
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

	fileText.reserve(fileSize);
	fileText.insert(fileText.begin(),
					std::istream_iterator<char>(file),
					std::istream_iterator<char>());
}

void Assembler::ReplaceText(std::string& s, const std::regex& expression, const std::string& value) const
{
	s = std::regex_replace(s, expression, value);
}

bool Assembler::HasMatch(const std::string& s, const std::regex& expression) const
{
	return std::regex_match(s, expression);
}

std::string Assembler::GetMatch(const std::string& s, const std::regex& expression, const int32_t index) const
{
	std::smatch match;
	if (std::regex_search(s.begin(), s.end(), match, expression))
	{
		return match[index];
	}

	assertf(false, "Failed to find match for: %s at index %d", s.c_str(), index);
	return "";
}

std::string Assembler::ToLowercase(const std::string& s) const
{
	std::string s2 = s;
	std::transform(s2.begin(), s2.end(), s2.begin(), tolower);
	return std::move(s2);
}
