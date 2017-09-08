#ifndef ASSOCIATIVE_CONFIG_HPP
#define ASSOCIATIVE_CONFIG_HPP

#include <boost/optional.hpp>
#include <boost/filesystem.hpp>

#include "gen/config.hpp"
#include "util.hpp"

namespace associative
{

	class Configuration
	{
	private:
		Configuration() = delete;
		Configuration(Configuration&) = delete;
		
	public:
		static boost::optional<uint64_t> maxLockTime();
		static bool debug();
		static std::string defaultIsolationLevel();
		static fs::path defaultLogPath();
	};

}

#endif