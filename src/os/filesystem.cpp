//
//	FileReader
//

#include "os/filesystem.h"

#include <assert.h>

FileReader::FileReader()
{

}

FileReader::~FileReader()
{
	Close();
}

bool FileReader::Open(const std::string& path, const int32_t mode)
{
	if (file.is_open())
	{
		printf("Failed to open file at path (%s)\n", path.c_str());
		return false;
	}

	file = std::ifstream(path, mode);

	if (file.is_open())
	{
		printf("Opened file at path (%s)\n", path.c_str());
		return true;
	}
	else
	{
		printf("Failed to open file at path (%s)\n", path.c_str());
		return false;
	}
}

bool FileReader::Exists(const std::string& path)
{
	if (Open(path))
	{
		printf("File exists at path (%s)\n", path.c_str());
		Close();
		return true;
	}
	else
	{
		printf("Failed to open file at path (%s)\n", path.c_str());
		return false;
	}
}

bool FileReader::Read(const int32_t length, std::string& out)
{
	if (file.is_open())
	{
		assert(out.size() >= length);
		file.read(&out[0], length);
		return true;
	}
	else
	{
		printf("Failed to read from file, file not open.\n");
		return false;
	}
}

bool FileReader::ReadLines(std::vector<std::string>& out)
{
	out.clear();
	if (file.is_open())
	{
		std::string s;
		while (getline(file, s))
		{
			out.push_back(s);
		}

		return true;
	}
	else
	{
		printf("Failed to read from file, file not open.\n");
		return false;
	}
}

int32_t FileReader::GetLength()
{
	int32_t len = -1;
	if (file.is_open())
	{
		file.seekg(0, std::ios::end);
		len = static_cast<int32_t>(file.tellg());
		file.seekg(0, std::ios::beg);
	}
	else
	{
		printf("Failed to get length from file, file not open.\n");
	}
	return len;
}

void FileReader::Close()
{
	if (file.is_open())
	{
		printf("Closing file.\n");
		file.close();
	}
	else
	{
		printf("Failed to close file, file not open.\n");
	}
}

FileWriter::FileWriter()
{

}

FileWriter::~FileWriter()
{
	Close();
}

bool FileWriter::Open(const std::string& path)
{
	if (file.is_open())
	{
		printf("Failed to open file at path (%s)\n", path.c_str());
		return false;
	}

	file = std::ofstream(path, std::ofstream::out);

	if (file.is_open())
	{
		printf("Opened file at path (%s)\n", path.c_str());
		return true;
	}
	else
	{
		printf("Failed to open file at path (%s)\n", path.c_str());
		return false;
	}
}

bool FileWriter::Exists(const std::string& path)
{
	if (Open(path))
	{
		printf("File exists at path (%s)\n", path.c_str());
		Close();
		return true;
	}
	else
	{
		printf("Failed to open file at path (%s)\n", path.c_str());
		return false;
	}
}

void FileWriter::Close()
{
	if (file.is_open())
	{
		printf("Closing file.\n");
		file.close();
	}
	else
	{
		printf("Failed to close file, file not open.\n");
	}
}

bool FileWriter::Write(const std::string& out)
{
	if (file.is_open())
	{
		file.write(&out[0], out.size());
		return true;
	}
	else
	{
		printf("Failed to write from file, file not open.\n");
		return false;
	}
}