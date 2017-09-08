#ifndef ASSOCIATIVE_IO_HPP
#define ASSOCIATIVE_IO_HPP

#include <algorithm>
#include <iostream>
#include <fstream>
#include <iterator>

#include "util.hpp"

namespace associative
{
	
	template<typename _CharT = char>
	void copyStreams(std::basic_istream<_CharT>& istream, std::basic_ostream<_CharT>& ostream)
	{
		std::ostreambuf_iterator<_CharT> output(ostream);
		std::istreambuf_iterator<_CharT> input(istream), end;
		std::copy(input, end, output);
	}
	
	template<typename _CharT = char>
	void readFile(const fs::path& path, std::basic_ostream<_CharT>& ostream)
	{
		std::basic_ifstream<_CharT> istream(path.string(), std::ios_base::binary);
		copyStreams<_CharT>(istream, ostream);
	}
	
	template<typename _CharT = char>
	void storeFile(const fs::path& path, std::basic_istream<_CharT>& istream)
	{
		fs::create_directories(path.parent_path());
		std::basic_ofstream<_CharT> ostream(path.string(), std::ios_base::binary);
		copyStreams<_CharT>(istream, ostream);
	}
	
	void createEmptyFile(const fs::path& path);
	
	class Line
	{
		// based on <http://stackoverflow.com/questions/1567082/how-do-i-iterate-over-cin-line-by-line-in-c/1567703#1567703>
		
	private:
		std::string data;
	
	public:
		friend std::istream& operator>>(std::istream&, Line&);
		operator std::string() const;
	};
	
	std::istream& operator>>(std::istream& istream, Line& line);
	
	template<typename _CharT, typename Func>
	void doREPL(const std::vector<std::basic_string<_CharT> >& input, const Func& func, const std::string& prompt = "> ", bool allowExit = true)
	{
		if (input.empty())
		{
			// TODO find out behaviour cout/istream_iterator(cin)
			std::cout << prompt << std::flush;
			std::istream_iterator<Line, _CharT> start(std::cin), end;
			for (auto iter = start; iter != end; ++iter)
			{
				std::string line = *iter;
				if (allowExit && line == ":exit")
					break;
				func(*iter);
				std::cout << prompt << std::flush;
			}
		}
		else
		{
			for (auto iter = input.begin(); iter != input.end(); ++iter)
				func(*iter);
		}
	}
	
}

#endif