#include "action.hpp"
#include "../util/util.hpp"
#include "../util/config.hpp"

#include "gen/action_impls.hpp"

using namespace po;

associative::MainParameters::MainParameters(const Parameters& parent, const variables_map& options, const std::vector<std::string>& unrecognized)
: Parameters(parent), options(options), unrecognized(unrecognized)
{
}

boost::optional<associative::ActionParameters> associative::MainParameters::parseFurther()
{
	return ActionParameters::fromCommandLine(unrecognized);
}

boost::optional<associative::MainParameters> associative::MainParameters::fromCommandLine(const std::vector<std::string>& args)
{
	ASSOCIATIVE_ACTIONS_INIT;
	
	auto parameters = Parameters::fromCommandLine(args);
	if (!parameters)
		return boost::none;
	
	options_description desc("Standard options");
	desc.add_options()
		("isolation-level", value<std::string>()->default_value(Configuration::defaultIsolationLevel()), "isolation level")
		("clear-shm", "debug option: clear shared memory");
	
	auto pair = Parameters::parseCommandLine(parameters->second, desc);
	
	return MainParameters(parameters->first, pair.first, pair.second);
}

associative::Action::Action(const std::string& name)
: Module(name)
{
}

associative::ModuleManager<associative::Action>& associative::Action::manager()
{
	static auto manager = new ModuleManager<Action>();
	return *manager;
}

associative::ActionParameters::ActionParameters(Action* const action, const variables_map& options)
: action(action), options(options)
{
}

int associative::ActionParameters::dispatch(associative::Environment& env)
{
	return action->perform(options, options["additional"].as<std::vector<std::string> >(), env);
}

boost::optional<associative::ActionParameters> associative::ActionParameters::fromCommandLine(std::vector<std::string> args)
{
	std::string name = args.at(0);
	if (!containsKey(Action::manager().modules(), name))
		throw formatException(boost::format("action %1% doesn't exist") % name);
	auto action = Action::manager().modules()[name];
	args.erase(args.begin());
	
	auto subDesc = action->desc();
	auto pair = Parameters::parseCommandLine(args, *subDesc);
	delete subDesc;
	
	return ActionParameters(action, pair.first);
}
