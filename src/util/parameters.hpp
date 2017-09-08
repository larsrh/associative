#ifndef ASSOCIATIVE_PARAMETERS_HPP
#define ASSOCIATIVE_PARAMETERS_HPP

#include <boost/optional.hpp>

#include "util.hpp"
#include "log.hpp"

namespace associative
{
	
	class Parameters
	{
	public:
		const std::string dataSource;
		const fs::path target;
		const fs::path log;
		
		const boost::shared_ptr<Logger> logger;
		
		Parameters(const std::string& dataSource, const fs::path& target, const fs::path& log);
		Parameters(const Parameters& other);
		virtual ~Parameters();
		
		static std::vector<std::string> argvToVector(int argc, char** argv);
		static boost::optional<std::pair<Parameters, std::vector<std::string> > > fromCommandLine(const std::vector<std::string>& args);
		
		static std::pair<po::variables_map, std::vector<std::string> > parseCommandLine(const std::vector<std::string>& args, po::options_description& desc);
	};
	
}

#endif