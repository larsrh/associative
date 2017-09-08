#include "io.hpp"

void associative::createEmptyFile(const fs::path& path)
{
	fs::create_directories(path.parent_path());
	std::ofstream(path.string(), std::ios_base::trunc);
}

std::istream& associative::operator>>(std::istream& istream, associative::Line& line)
{
	std::getline(istream, line.data);
	return istream;
}

associative::Line::operator std::string() const
{
	return data;
}