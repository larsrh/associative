#include <iostream>

#include "parameters.hpp"
#include "config.hpp"

using namespace po;

associative::Parameters::Parameters(const std::string& dataSource, const fs::path& target, const fs::path& log)
: dataSource(dataSource), target(target), log(log), logger(new Logger(log))
{
}

associative::Parameters::Parameters(const associative::Parameters& other)
: dataSource(other.dataSource), target(other.target), log(other.log), logger(other.logger)
{
}

associative::Parameters::~Parameters()
{
}

std::vector<std::string> associative::Parameters::argvToVector(int argc, char** argv)
{
	std::vector<std::string> args(argc);
	for (int i = 0; i < argc; ++i)
		args[i] = argv[i];
	return args;
}

boost::optional<std::pair<associative::Parameters, std::vector<std::string> > > associative::Parameters::fromCommandLine(const std::vector<std::string>& args)
{
	options_description desc;
	desc.add_options()
		("data-source", value<std::string>(), "data source (format: [provider]:[file/host/etc.])")
		("target", value<fs::path>()->default_value(".", "current working directory"), "target directory for storage")
		("log", value<fs::path>()->default_value(Configuration::defaultLogPath()), "log file");
	
	auto pair = parseCommandLine(args, desc);
	auto& vm = pair.first;
	
	if (vm.count("help") || !vm.count("data-source"))
	{
		std::cerr << desc << std::endl;
		return boost::none;
	}
	
	return boost::make_optional(
		std::make_pair(Parameters(
			vm["data-source"].as<std::string>(),
			vm["target"].as<fs::path>(),
			vm["log"].as<fs::path>()
		), pair.second)
	);
}

std::pair<variables_map, std::vector<std::string> > associative::Parameters::parseCommandLine(const std::vector<std::string>& args, options_description& desc)
{
	options_description additional;
	additional.add_options()
		("additional", value<std::vector<std::string> >()->default_value(std::vector<std::string>(), ""));
	
	positional_options_description po;
	po.add("additional", -1);
	
	variables_map vm;
	parsed_options parsed = command_line_parser(args).options(desc.add(additional)).positional(po).allow_unregistered().run();
	store(parsed, vm);
	notify(vm);
	
	return std::make_pair(vm, collect_unrecognized(parsed.options, include_positional));
}
