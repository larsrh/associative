#include "../action.hpp"
#include "../../objects/file.hpp"

using namespace po;

namespace associative
{
	
	class LsAction : public Action
	{
		COMMANDLINE_DECL;
		
	protected:
		virtual options_description* desc()
		{
			auto desc = new options_description("ls options");
			desc->add_options()
				("uuid", value<std::string>(), "UUID of the file");
			return desc;
		}
		
	public:
		LsAction()
		: Action("ls")
		{
		}
		
		virtual int perform(const variables_map& vm, const std::vector<std::string>&, Environment& env)
		{
			if (!vm.count("uuid"))
				return 1;
			
			auto file = env.getFile(vm["uuid"].as<std::string>());
			std::cout << collToString(file->getBlobNames(), SimpleCollFormat<std::set<std::string> >("", "\n", "\n"));
			
			return 0;
		}

	};
	
}

COMMANDLINE_DEF(Ls);
