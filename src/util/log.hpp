#ifndef ASSOCIATIVE_LOG_HPP
#define ASSOCIATIVE_LOG_HPP

#include <log4cpp/Appender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/Priority.hh>

#include "util.hpp"

#define TRACE(LOGGER, MSG) \
	LOGGER->debug() << __FILE__ << ":" << __LINE__ << " " << (MSG);

namespace associative
{
	
	class Logger
	{
	private:
		log4cpp::Appender* const appender;
		log4cpp::Layout* const layout;
		log4cpp::Category& category;
	
	public:
		typedef log4cpp::Priority Priority;
		typedef log4cpp::Priority::PriorityLevel PriorityLevel;
		
		Logger(const fs::path& file);
		~Logger();
		
		log4cpp::CategoryStream log(const PriorityLevel& level);
		
		log4cpp::CategoryStream error();
		log4cpp::CategoryStream warn();
		log4cpp::CategoryStream info();
		log4cpp::CategoryStream debug();
	};
	
}

#endif