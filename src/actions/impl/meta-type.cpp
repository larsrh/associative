#include "../action.hpp"
#include "../../objects/type.hpp"

using namespace po;

namespace associative
{
	
	class MetaTypeAction : public Action
	{
		COMMANDLINE_DECL;
		
	protected:
		virtual options_description* desc()
		{
			auto desc = new options_description("meta-type options");
			desc->add_options()
				("type-name", value<std::string>(), "short name of the type")
				("prefix-name", value<std::string>(), "short name of the corresponding prefix");
			return desc;
		}
		
	public:
		MetaTypeAction()
		: Action("meta-type")
		{
		}
		
		virtual int perform(const variables_map& vm, const std::vector<std::string>&, Environment& env)
		{
			if (!vm.count("prefix-name") || !vm.count("type-name"))
				return 1;
			
			auto prefix = Prefix::get(env.getConnection(), vm["prefix-name"].as<std::string>());
			Type::get(env.getConnection(), vm["type-name"].as<std::string>(), prefix);
			return 0;
		}

	};
	
}

COMMANDLINE_DEF(MetaType);
