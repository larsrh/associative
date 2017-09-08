#include "../action.hpp"
#include "../../objects/file.hpp"
#include "../../util/io.hpp"

using namespace po;

namespace associative
{

	class CatAction : public Action
	{
		COMMANDLINE_DECL;
		
	protected:
		virtual options_description* desc()
		{
			auto desc = new options_description("cat options");
			desc->add_options()
				("uuid", value<std::string>(), "UUID of the file")
				("blob-name", value<std::string>(), "Name of the blob");
			return desc;
		}
		
	public:
		CatAction()
		: Action("cat")
		{
		}
		
		virtual int perform(const po::variables_map& vm, const std::vector<std::string>&, Environment& env)
		{
			if (!vm.count("uuid") || !vm.count("blob-name"))
				return 1;
			
			auto file = env.getFile(vm["uuid"].as<std::string>());
			readFile(file->getBlob(vm["blob-name"].as<std::string>())->getPath(), std::cout);
			return 0;
		}
	};

}

COMMANDLINE_DEF(Cat);
