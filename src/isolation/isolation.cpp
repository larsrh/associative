#include "isolation.hpp"
#include "../util/config.hpp"

#include "gen/isolevel_impls.hpp"

associative::IsolationLevel::IsolationLevel(const std::string& name)
: Module(name)
{
}

associative::ModuleManager<associative::IsolationLevel>& associative::IsolationLevel::manager()
{
	static auto manager = new ModuleManager<IsolationLevel>();
	return *manager;
}

const associative::IsolationLevel& associative::IsolationLevel::getIsolationLevel(const boost::optional<std::string>& level)
{
	ASSOCIATIVE_ISOLEVEL_INIT;
	
	auto l = level.get_value_or(Configuration::defaultIsolationLevel());
	
	if (!containsKey(manager().modules(), l))
	{
		if (level)
			throw formatException(boost::format("%1% is not a valid isolation level") % l);
		else
			throw formatException(boost::format("%1% is the default isolation level but not existing, check configuration") % l);
	}
	
	return *manager().modules()[l];
}
