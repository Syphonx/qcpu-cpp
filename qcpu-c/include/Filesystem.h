//
//	FileReader
//

#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

enum class EFileType : uint8_t
{
	Binary,
	Text
};

class FileReader
{
public:
	FileReader() = default;
	~FileReader();

	bool Open(const std::string& path, int32_t mode = std::ios::in);
	bool Exists(const std::string& path);
	void Close();

	std::ifstream& GetStream()
	{
		return file;
	}

	bool Read(int32_t length, std::string& out);
	bool ReadLines(std::vector<std::string>& out);
	int32_t GetLength();

private:
	std::ifstream file;
};

class FileWriter
{
public:
	FileWriter() = default;
	~FileWriter();

	bool Open(const std::string& path);
	bool Exists(const std::string& path);
	void Close();

	std::ofstream& GetStream()
	{
		return file;
	}

	bool Write(const std::string& out);

private:
	std::ofstream file;
};
