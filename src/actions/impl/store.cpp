#include "../action.hpp"
#include "../../objects/file.hpp"
#include "../../util/io.hpp"

using namespace po;

namespace associative
{

	class StoreAction : public Action
	{
		COMMANDLINE_DECL;
		
	protected:
		virtual options_description* desc()
		{
			auto desc = new options_description("store options");
			desc->add_options()
				("uuid", value<std::string>(), "UUID of the file")
				("blob-name", value<std::string>(), "Name of the blob")
				("content-type", value<std::string>()->default_value("text/plain"), "Content type");
			return desc;
		}
		
	public:
		StoreAction()
		: Action("store")
		{
		}
		
		virtual int perform(const po::variables_map& vm, const std::vector<std::string>&, Environment& env)
		{
			if (!vm.count("uuid") || !vm.count("blob-name"))
				return 1;
			
			auto file = env.getFile(vm["uuid"].as<std::string>());
			
			std::string name = vm["blob-name"].as<std::string>();
			if (!file->hasBlob(name))
				file->addBlob(name, vm["content-type"].as<std::string>());
			
			storeFile(file->getBlob(name)->getPath(true), std::cin);
			return 0;
		}
	};

}

COMMANDLINE_DEF(Store);
