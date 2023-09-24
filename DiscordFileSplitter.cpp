#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "Types.hpp"

#define DISCORD_FILE_SIZE 0x1900000

#define FILE_SIZE_ARG 's'
#define FILE_COUNT_ARG 'c'
#define FILE_INPUT_ARG 'i'
#define FILE_OUTPUT_ARG 'o'
#define FILE_ASSEMBLE_ARG 'a'

struct Parameters
{
	std::filesystem::path source;
	std::filesystem::path destination;
	u64 maxFileSize = DISCORD_FILE_SIZE;
	u64 fileCount = 0;
	bool assembleFile = false;
};

std::string ToString(u64 number, u32 minChars)
{
	std::string result;
	result.resize(minChars);
	std::lldiv_t res = {};
	for (int i = minChars-1; i >= 0; i--)
	{
		res = std::lldiv(number, 10);
		number = res.quot;
		result[i] = '0' + static_cast<u8>(res.rem);
	}
	if (res.quot) result.append("INVALID");
	return result;
}

u32 GetTotalFileCount(u64 sizeIn, u64 maxSize)
{
	std::lldiv_t res = std::lldiv(sizeIn, maxSize);
	u32 result = static_cast<u32>(res.quot) + (res.rem != 0);
	return result;
}

u32 GetCharCount(u64 input)
{
	return static_cast<u32>(std::floor(std::log10(input-1) + 1));
}

bool ReadInteger(u64& i, char const* s)
{
	std::stringstream ss(s);
	s64 res;
	ss >> res;
	if (ss.fail() || res <= 0)
	{
		return false;
	}
	i = res;
	return true;
}

bool ParseArgs(int argc, char* argv[], Parameters& params)
{
	if (argc == 2)
	{
		std::filesystem::path p(argv[1]);
		if (!std::filesystem::exists(p))
		{
			p = std::filesystem::current_path().append(argv[1]);
			if (!std::filesystem::exists(p))
			{
				std::cerr << "Invalid file " << argv[1] << "!" << std::endl;
				return false;
			}
		}
		return true;
	}
	for (s32 i = 1; i < argc - 1; ++i)
	{
		if (argv[i][0] != '-' && argv[i][0] != '/') continue;
		std::filesystem::path p;
		switch (argv[i][1])
		{
		case FILE_SIZE_ARG:
			if (!ReadInteger(params.maxFileSize, argv[i + 1]))
			{
				params.maxFileSize = 0;
				if (strcmp(argv[i + 1], "0"))
				{
					std::cerr << "Invalid number " << argv[i + 1] << "!" << std::endl;
				}
			}
			else
			{
				std::cout << "Set max file size to " << params.maxFileSize << std::endl;
			}
			++i;
			break;
		case FILE_COUNT_ARG:
			if (!ReadInteger(params.fileCount, argv[i + 1]))
			{
				params.fileCount = 0;
				if (strcmp(argv[i + 1], "0"))
				{
					std::cerr << "Invalid number " << argv[i + 1] << "!" << std::endl;
				}
			}
			else
			{
				std::cout << "Set file count to " << params.fileCount << std::endl;
			}
			++i;
			break;
		case FILE_INPUT_ARG:
			p = std::filesystem::path(argv[i + 1]);
			if (!std::filesystem::exists(p))
			{
				p = std::filesystem::current_path().append(argv[i + 1]);
				if (!std::filesystem::exists(p))
				{
					std::cerr << "Invalid file " << argv[i + 1] << "!" << std::endl;
					return false;
				}
			}
			params.source = p;
			++i;
			break;
		case FILE_OUTPUT_ARG:
			p = std::filesystem::path(argv[i + 1]);
			if (!std::filesystem::exists(p.parent_path()))
			{
				p = std::filesystem::current_path().append(argv[i + 1]);
				if (!std::filesystem::exists(p.parent_path()))
				{
					std::cerr << "Invalid destination " << argv[i + 1] << "!" << std::endl;
					return false;
				}
			}
			params.destination = p;
			++i;
			break;
		case FILE_ASSEMBLE_ARG:
			params.assembleFile = true;
			break;
		default:
			break;
		}
	}
	return true;
}

size_t TryOpen(std::filesystem::path file, std::ifstream& in)
{
	in.open(file, std::ios::binary | std::ios::in | std::ios::ate);
	if (!in.is_open())
	{
#ifdef _WIN32
		char result[256];
		strerror_s(result, 256, errno);
		std::cout << "Could not open file " << file << " : Error code " << result << std::endl;
#else
		std::cout << "Could not open file " << file << " : Error code " << strerror(errno) << std::endl;
#endif
		return 0;
	}
	size_t len = in.tellg();
	in.seekg(0, in.beg);
	return len;
}

void HandleSplit(Parameters& params)
{
	std::ifstream in;
	size_t filelength = TryOpen(params.source, in);
	if (filelength == 0) return;
	if (params.destination.empty())
	{
		params.destination = std::filesystem::current_path().append(params.source.filename().string());
	}
	if (params.fileCount != 0)
	{
		std::lldiv_t res = std::lldiv(filelength, params.fileCount);
		params.maxFileSize = res.quot + (res.rem != 0);
	}
	else
	{
		params.fileCount = GetTotalFileCount(filelength, params.maxFileSize);
	}
	u32 charCount = GetCharCount(params.fileCount);
	char* data = new char[filelength];
	in.read(data, filelength);
	in.close();
	for (size_t i = 0; i < params.fileCount; i++)
	{
		size_t delta = i * params.maxFileSize;
		size_t sz;
		if (delta + params.maxFileSize > filelength)
		{
			sz = filelength - delta;
		}
		else
		{
			sz = params.maxFileSize;
		}
		std::ofstream out;
		out.open(params.destination.string().append(".part" + ToString(i, charCount)), std::ios::binary | std::ios_base::trunc | std::ios::beg);
		out.write(data + delta, sz);
		out.close();
	}
	delete[] data;
}

void HandleFuse(Parameters& params)
{
	std::string s = params.source.extension().string();
	u64 res = s.find("part");
	if (res <= 0 || res > 64) return;
	std::string number = s.substr(5);
	u32 charCount = static_cast<u32>(number.size());
	if (charCount > 64) return;
	std::string cleanName = params.source.string();
	cleanName = cleanName.substr(0, cleanName.size() - (5 + charCount));
	if (params.destination.empty())
	{
		params.destination = cleanName;
	}
	cleanName.append(".part");
	int current = 0;
	std::ofstream out;
	out.open(params.destination, std::ios::binary | std::ios_base::trunc | std::ios::beg);
	while (true)
	{
		std::filesystem::path f = std::string(cleanName).append(ToString(current, charCount));
		if (!std::filesystem::exists(f)) break;
		std::ifstream in;
		u64 len = TryOpen(f, in);
		if (len == 0) break;
		current++;
		char* data = new char[len];
		in.read(data, len);
		in.close();
		out.write(data, len);
	}
	out.close();
}

int main(int argc, char** argv)
{
	Parameters params;
	ParseArgs(argc, argv, params);

	if (params.assembleFile)
	{
		HandleFuse(params);
	}
	else
	{
		HandleSplit(params);
	}
	
	return 0;
}