#include "../action.hpp"
#include "../../objects/file.hpp"

using namespace po;

namespace associative
{
	
	class RmAction : public Action
	{
		COMMANDLINE_DECL;
		
	protected:
		virtual options_description* desc()
		{
			auto desc = new options_description("rm options");
			desc->add_options()
				("uuid", value<std::string>(), "UUID of the file")
				("blob-name", value<std::vector<std::string> >(), "Name of the blob to remove");
			return desc;
		}
		
	public:
		RmAction()
		: Action("rm")
		{
		}
		
		virtual int perform(const variables_map& vm, const std::vector<std::string>&, Environment& env)
		{
			if (!vm.count("uuid"))
				return 1;
			
			auto file = env.getFile(vm["uuid"].as<std::string>());
			
			if (vm.count("blob-name"))
			{
				auto& blobs = vm["blob-name"].as<std::vector<std::string> >();
				for (auto iter = blobs.begin(); iter != blobs.end(); ++iter)
					file->removeBlob(*iter);
			}
			else
			{
				file->removeFile();
			}
			
			return 0;
		}

	};
	
}

COMMANDLINE_DEF(Rm);
