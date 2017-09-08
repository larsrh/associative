#include "../action.hpp"
#include "../../objects/prefix.hpp"

using namespace po;

namespace associative
{
	
	class MetaPrefixAction : public Action
	{
		COMMANDLINE_DECL;
		
	protected:
		virtual options_description* desc()
		{
			auto desc = new options_description("meta-prefix options");
			desc->add_options()
				("name", value<std::string>(), "short name of the prefix")
				("uri", value<std::string>(), "URI of the prefix");
			return desc;
		}
		
	public:
		MetaPrefixAction()
		: Action("meta-prefix")
		{
		}
		
		virtual int perform(const variables_map& vm, const std::vector<std::string>&, Environment& env)
		{
			if (!vm.count("name"))
				return 1;
			
			auto name = vm["name"].as<std::string>();
			if (vm.count("uri"))
				Prefix::get(env.getConnection(), name, boost::make_optional(vm["uri"].as<std::string>()));
			else
				std::cout << Prefix::get(env.getConnection(), name, boost::none)->uri << std::endl;
			
			return 0;
		}

	};
	
}

COMMANDLINE_DEF(MetaPrefix);
