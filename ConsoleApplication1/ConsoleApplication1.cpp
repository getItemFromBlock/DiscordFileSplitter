#include <iostream>
#include <filesystem>
#include <fstream>

#define DISCORD_MAX_FILE 0x1900000

std::string getString(size_t number)
{
	std::string result;
	do
	{
		auto res = std::lldiv(number, 10llu);
		number = res.quot;
		result.push_back('0' + res.rem);
	} while (number);
	return result;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "Please input a file" << std::endl;
        return 0;
    }
    std::filesystem::path p(argv[1]);
    if (!std::filesystem::exists(p))
    {
		p = std::filesystem::current_path().append(argv[1]);
		if (!std::filesystem::exists(p))
		{
			std::cout << "Invalid file " << argv[1] << "!" << std::endl;
			return 0;
		}
    }
	std::ifstream in;
	in.open(p, std::ios::binary | std::ios::in | std::ios::ate);
	if (!in.is_open())
	{
#ifdef _WIN32
		char result[256];
		strerror_s(result, 256, errno);
		std::cout << "Could not open file " << p << " : Error code " << result << std::endl;
#else
		std::cout << "Could not open file " << p << " : Error code " << strerror(errno) << std::endl;
#endif
		return 0;
	}
	size_t filelength = in.tellg();
	in.seekg(0, in.beg);

	char* data = new char[filelength];
	in.read(data, filelength);
	in.close();
	for (size_t i = 0; i < filelength; i += DISCORD_MAX_FILE)
	{
		size_t sz = i + DISCORD_MAX_FILE <= filelength ? DISCORD_MAX_FILE : filelength - i;
		std::ofstream out;
		out.open(p.filename().string().append(".part" + getString(i/DISCORD_MAX_FILE)), std::ios::binary | std::ios_base::trunc | std::ios::beg);
		out.write(data + i, sz);
		out.close();
	}
	delete[] data;
	return 0;
}