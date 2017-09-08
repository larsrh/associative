#include <string>

#include "exception.hpp"

associative::Exception::Exception()
: message("")
{
}

associative::Exception::Exception(const std::string& message)
: message(message)
{
}

associative::Exception::~Exception() throw()
{
}

std::string associative::Exception::getMessage() const throw()
{
	return this->message;
}

const char* associative::Exception::what() const throw()
{
    return this->message.c_str();
}
