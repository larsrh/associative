#ifndef ASSOCIATIVE_EXCEPTION_HPP
#define ASSOCIATIVE_EXCEPTION_HPP

#include <exception>

#include <boost/format.hpp>

namespace associative
{
	
	class Exception : public std::exception
	{
	private:
		std::string message;
		
	public:
		Exception();
		Exception(const std::string& message);
		~Exception() throw();
		
		virtual const char* what() const throw();
		virtual std::string getMessage() const throw();
	};
	
	template<typename E = Exception, typename... Args>
	E formatException(const boost::basic_format<char>& format, Args... args)
	{
		return E(format.str(), args...);
	}
	
}

#endif