//
//	FileReader
//

#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

enum class EFileType
{
	Binary,
	Text
};

class FileReader
{

public:

	FileReader();
	~FileReader();

	bool						Open(const std::string& path, const int32_t mode = std::ios::in);
	bool						Exists(const std::string& path);
	void						Close();
	std::ifstream&				GetStream()
	{
		return file;
	}

	bool						Read(const int32_t length, std::string& out);
	bool						ReadLines(std::vector<std::string>& out);
	int32_t						GetLength();

	std::ifstream				file;
};

class FileWriter
{

public:

	FileWriter();
	~FileWriter();


	bool						Open(const std::string& path);
	bool						Exists(const std::string& path);
	void						Close();
	std::ofstream&				GetStream()
	{
		return file;
	}

	bool						Write(const std::string& out);

	std::ofstream				file;
};