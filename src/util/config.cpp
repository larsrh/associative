#include <boost/lexical_cast.hpp>

#include "config.hpp"

boost::optional<uint64_t> associative::Configuration::maxLockTime()
{
	auto parsed = boost::lexical_cast<int64_t>(ASSOCIATIVE_MAX_LOCK_TIME);
	if (parsed <= 0)
		return boost::none;
	else
		return boost::optional<uint64_t>(parsed);
}

bool associative::Configuration::debug()
{
#ifdef ASSOCIATIVE_DEBUG
	return true;
#else
	return false;
#endif
}

std::string associative::Configuration::defaultIsolationLevel()
{
	return ASSOCIATIVE_DEFAULT_ISOLEVEL;
}

boost::filesystem3::path associative::Configuration::defaultLogPath()
{
	return boost::filesystem3::path(ASSOCIATIVE_DEFAULT_LOG);
}
