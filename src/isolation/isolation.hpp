#ifndef ASSOCIATIVE_ISOLATION_HPP
#define ASSOCIATIVE_ISOLATION_HPP

#include "../util/modules.hpp"
#include "../db/connection.hpp"

#include "gen/isolevel_impls.hpp"

#define ISOLEVEL_DECL \
	MODULE_DECL

#define ISOLEVEL_DEF(NAME) \
	MODULE_DEF(NAME, Isolation, IsolationLevel) \
	const associative::IsolationLevel& associative::IsolationLevels::NAME = associative::NAME##Isolation();

namespace associative
{
	
	class IsolationLevel : public Module
	{
	protected:
		IsolationLevel(const std::string& name);
		
		static ModuleManager<IsolationLevel>& manager();
	
	public:
		virtual bool isIsolated(const boost::shared_ptr<Connection>& conn, uint64_t sessionID) const = 0;
		
		static const IsolationLevel& getIsolationLevel(const boost::optional<std::string>& level = boost::none);
	};
	
}

#endif