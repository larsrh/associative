#ifndef ASSOCIATIVE_ACTION_HPP
#define ASSOCIATIVE_ACTION_HPP

#include <string>
#include <vector>
#include <map>

#include <boost/program_options.hpp>
#include <boost/optional.hpp>

#include "../util/util.hpp"
#include "../util/parameters.hpp"
#include "../util/modules.hpp"
#include "../env/environment.hpp"

#define COMMANDLINE_DECL \
	MODULE_DECL

#define COMMANDLINE_DEF(NAME) \
	MODULE_DEF(NAME, Action, Action)

namespace associative
{
	
	class Environment;
	class ActionParameters;
	
	class MainParameters : public Parameters
	{
	private:
		MainParameters(const Parameters& parent, const po::variables_map& options, const std::vector<std::string>& unrecognized);
		
	public:
		const po::variables_map options;
		const std::vector<std::string> unrecognized;
		
		boost::optional<ActionParameters> parseFurther();
		
		static boost::optional<MainParameters> fromCommandLine(const std::vector<std::string>& args);
	};
	
	class Action : public Module
	{
		friend class ActionParameters;
		
	protected:
		static ModuleManager<Action>& manager();
		
		Action(const std::string& name);
		
		virtual po::options_description* desc() = 0;
		
	public:
		virtual int perform(const po::variables_map& vm, const std::vector<std::string>& parameters, Environment& env) = 0;
	};
	
	class ActionParameters
	{
	private:
		ActionParameters(Action* const action, const po::variables_map& options);
		
	public:
		Action* const action;
		const po::variables_map options;
		
		int dispatch(Environment& env);
		
		static boost::optional<ActionParameters> fromCommandLine(std::vector<std::string> args);
	};
	
}

#endif