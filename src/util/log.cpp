#include <log4cpp/FileAppender.hh>
#include <log4cpp/BasicLayout.hh>

#include "log.hpp"

associative::Logger::Logger(const fs::path& file)
: appender(new log4cpp::FileAppender("FileAppender", file.c_str())),
  layout(new log4cpp::BasicLayout()),
  category(log4cpp::Category::getInstance("associative-fs"))
{
	category.addAppender(appender);
	appender->setThreshold(Priority::DEBUG);
	category.setPriority(Priority::DEBUG);
}

associative::Logger::~Logger()
{
	category.removeAppender(appender);
	delete layout;
	
	// deleting appender leads to a segfault, so we just close it here
	//appender->close();
	// Hey, even closing segfaults!
}

log4cpp::CategoryStream associative::Logger::log(const PriorityLevel& level)
{
	return category << level;
}

log4cpp::CategoryStream associative::Logger::error()
{
	return log(Priority::ERROR);
}

log4cpp::CategoryStream associative::Logger::warn()
{
	return log(Priority::WARN);
}

log4cpp::CategoryStream associative::Logger::info()
{
	return log(Priority::INFO);
}

log4cpp::CategoryStream associative::Logger::debug()
{
	return log(Priority::DEBUG);
}
